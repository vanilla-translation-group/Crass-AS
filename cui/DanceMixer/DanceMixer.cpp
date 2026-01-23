#include <windows.h>
#include <tchar.h>
#include <crass_types.h>
#include <acui.h>
#include <cui.h>
#include <package.h>
#include <resource.h>
#include <cui_error.h>
#include <cui_common.h>
#include <stdio.h>
#include <zlib.h>

// For Time Leap Paradise.

/* 接口数据结构: 表示cui插件的一般信息 */
struct acui_information DanceMixer_cui_information = {
	_T("Front Wing"),	/* copyright */
	_T(""),	/* system */
	_T(".pac"),	/* package */
	_T("1.0.0"),	/* revision */
	_T("ほしうた"),	/* author */
	_T("2026-01-22 14:21"),	/* date */
	NULL,	/* notion */
	ACUI_ATTRIBUTE_LEVEL_UNSTABLE
};

/* 所有的封包特定的数据结构都要放在这个#pragma段里 */
#pragma pack (1)

typedef struct {
	s8 magic[8];
	u32 unknown1;
	u32 unknown2;
	u32 count;
	u32 unknown3;
	u8 flag;
	u8 maxdec;
	s8 zero[6];
} pac_header_t;

typedef struct {
	s8 name[0x104];
	u32 offset;
	u32 uncompr_length;
	u32 length;
} pac_entry_t;

#pragma pack ()

#define ROTR8(x, r) ((x >> r) | (x << (8 - r)))

static void decode(void *data, int length, pac_header_t *header) {
	u8 *dat = (u8 *)data;
	switch (header->flag) {
		case 0: {
			u8 key = 0xcb;
			for (int i = 0; i < length; i++) {
				dat[i] ^= key;
				dat[i] = ROTR8(dat[i], 1);
				key = 1; // key != 0 || (key & 0xfe) != 0
			}
			break;
		}
		// 下面这两种其实不会被用到，我也没有测试过。
		case 1: {
			char key[] = "68F9CC3FCF554b3f81D60C3ADA64C4FEABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ";
			for (int i = 0, j = 0; i < length; i++) {
				dat[i] ^= key[j + 0x20];
				j++;
				if (j >= header->maxdec || j >= sizeof(key) - 0x20)
					j = 0;
			}
			break;
		}
		case 2: {
			u8 key = 0xda;
			for (int i = 0; i < length; i++) {
				dat[i] ^= key;
				dat[i] = ROTR8(dat[i], 1);
				key = 2; // (key != 0 || (key & 0xfe) != 0) + 1
			}
			break;
		}
	}
}

/******************** pac ********************/

/* 封包匹配回调函数 */
static int DanceMixer_pac_match(struct package *pkg) {
	if (pkg->pio->open(pkg, IO_READONLY))
		return -CUI_EOPEN;

	pac_header_t *header = (pac_header_t *)malloc(sizeof(pac_header_t));
	if (!header)
		return -CUI_EMEM;
	
	if (pkg->pio->read(pkg, header, sizeof(*header))) {
		pkg->pio->close(pkg);
		free(header);
		return -CUI_EREAD;
	}

	if (strncmp(header->magic, "TLP_DAT", 7)) {
		pkg->pio->close(pkg);
		free(header);
		return -CUI_EMATCH;	
	}

	package_set_private(pkg, header);

	return 0;
}

/* 封包索引目录提取函数 */
static int DanceMixer_pac_extract_directory(struct package *pkg, struct package_directory *pkg_dir) {
	pac_header_t *header = (pac_header_t *)package_get_private(pkg);
	u32 index_length = header->count * sizeof(pac_entry_t);

	u8 *index = (u8 *)malloc(index_length);
	if (!index)
		return -CUI_EMEM;

	if (pkg->pio->read(pkg, index, index_length)) {
		free(index);
		return -CUI_EREAD;
	}

	decode(index, index_length, header);

	pkg_dir->directory = index;
	pkg_dir->directory_length = index_length;
	pkg_dir->index_entries = header->count;
	pkg_dir->index_entry_length = sizeof(pac_entry_t);
	// 第一个文件必须是 data/ajfkur3h45n56d7u78a7nh9u7iI8ny0fau6i4al27we4hfuelnrg，懒了不判断了

	return 0;
}

/* 封包索引项解析函数 */
static int DanceMixer_pac_parse_resource_info(struct package *pkg, struct package_resource *pkg_res) {
	pac_entry_t *pac_entry;

	pac_entry = (pac_entry_t *)pkg_res->actual_index_entry;
	strcpy(pkg_res->name, pac_entry->name);
	pkg_res->name_length = -1;			/* -1表示名称以NULL结尾 */
	pkg_res->raw_data_length = pac_entry->length;
	pkg_res->actual_data_length = pac_entry->uncompr_length;
	pkg_res->offset = pac_entry->offset;

	return 0;
}

/* 封包资源提取函数 */
static int DanceMixer_pac_extract_resource(struct package *pkg, struct package_resource *pkg_res) {
	u8 *raw = (u8 *)malloc(pkg_res->raw_data_length);
	if (!raw)
		return -CUI_EMEM;

	if (pkg->pio->readvec(pkg, raw, pkg_res->raw_data_length, pkg_res->offset, IO_SEEK_SET)) {
		free(raw);
		return -CUI_EREADVEC;
	}

	pac_header_t *header = (pac_header_t *)package_get_private(pkg);
	decode(raw, min((u32)(header->maxdec >> 1), pkg_res->raw_data_length), header);

	u8 *actual = (u8 *)malloc(pkg_res->actual_data_length);
	if (!actual) {
		free(raw);
		return -CUI_EMEM;
	}

	if (uncompress(actual, &pkg_res->actual_data_length, raw, pkg_res->raw_data_length) != Z_OK) {
		free(raw);
		free(actual);
		return -CUI_EUNCOMPR;
	}
	pkg_res->actual_data = actual;

	return 0;
}

/* 封包卸载函数 */
static void DanceMixer_pac_release(struct package *pkg, struct package_directory *pkg_dir) {
	cui_common_release(pkg, pkg_dir);

	pac_header_t *header = (pac_header_t *)package_get_private(pkg);
	if (header) {
		free(header);
		package_set_private(pkg, NULL);
	}
} 

/* 封包处理回调函数集合 */
static cui_ext_operation DanceMixer_pac_operation = {
	DanceMixer_pac_match,			/* match */
	DanceMixer_pac_extract_directory,		/* extract_directory */
	DanceMixer_pac_parse_resource_info,	/* parse_resource_info */
	DanceMixer_pac_extract_resource,		/* extract_resource */
	cui_common_save_resource,
	cui_common_release_resource,
	DanceMixer_pac_release
};

int CALLBACK DanceMixer_register_cui(struct cui_register_callback *callback) {
	/* 注册cui插件支持的扩展名、资源放入扩展名、处理回调函数和封包属性 */

	if (callback->add_extension(callback->cui, _T(".pac"), NULL, NULL, &DanceMixer_pac_operation, CUI_EXT_FLAG_PKG | CUI_EXT_FLAG_DIR)) return -1;

	return 0;
}
