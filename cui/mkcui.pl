#!/usr/bin/env perl

use POSIX;

sub prompt {
	my $limited = $_[1] && ref $_[1] eq "ARRAY";
	while (1) {
		print $_[0];
		if ($limited) {
			print " (" . join("|", @{$_[1]}) . ")";
		}
		elsif ($_[1]) {
			print " [$_[1]]";
		}
		print " : ";
		my $input = <STDIN>;
		chomp $input;
		if ($limited) {
			if (grep $_ eq $input, @{$_[1]}) {
				return $input;
			}
		}
		else {
			return $input || $_[1];
		}
	}
}

sub ornull {
	if ((!$_[1] && defined($_[0])) || ($_[1] && $_[0])) {
		return "_T(\"$_[0]\")";
	}
	else {
		return "NULL";
	}
}

mkdir(@ARGV[0]) or die "Unable to create directory. Maybe plugin already exists?";

$converted = @ARGV[0];
$converted =~ s/\..*//g;
$converted =~ s/\+/p/g;
$converted =~ s/[ -]/_/g;
$converted =~ s/^(\d)/_$1/g;

$def = << "EOF";
LIBRARY "$converted"
EXPORTS
	${converted}_register_cui
	${converted}_cui_information
EOF

$copyright = ornull(prompt("版权", ""));
$system = ornull(prompt("系统", ""));
$package = "";
$revision = ornull(prompt("版本", ""));
$author = ornull(prompt("作者", ""));
$notion = ornull(prompt("注意"));
$level = prompt("状态", ["STABLE", "UNSTABLE", "DEVELOP"]);

$operations = "";
$registrations = "";

foreach $name (@ARGV[1..$#ARGV]) {
	@flaglist = ();
	$type = prompt("<$name> 是否为目录型", ["y", "n"]) eq "y";
	if ($type) {
		push(@flaglist, "CUI_EXT_FLAG_DIR");
	}
	$extension = prompt("<$name> 后缀名（为空则输入 .）", $name);
	if ($extension eq ".") {
		push(@flaglist, "CUI_EXT_FLAG_NOEXT");
		undef($extension);
	}
	else {
		$extension = ".$extension";
		$package .= "$extension ";
	}
	$extension = ornull($extension);
	if (!$type) {
		$replacement = "." . prompt("<$name> 替换用后缀名");
	}
	$replacement = ornull($replacement);
	$description = ornull(prompt("<$name> 格式描述"));
	$flag = join(" | ", @flaglist) || "0";
	$registrations .= << "EOF";

	if (callback->add_extension(callback->cui, $extension, $replacement, $description, &${converted}_${name}_operation, $flag)) return -1;
EOF

	$operations .= "\n/******************** $name ********************/\n\n";
	$operations .= << "EOF";
/* 封包匹配回调函数 */
static int ${converted}_${name}_match(struct package *pkg) {
}

EOF
	if ($type) {
		$operations .= << "EOF";
/* 封包索引目录提取函数 */
static int ${converted}_${name}_extract_directory(struct package *pkg, struct package_directory *pkg_dir) {
}

/* 封包索引项解析函数 */
static int ${converted}_${name}_parse_resource_info(struct package *pkg, struct package_resource *pkg_res) {
}

EOF
	}
	$operations .= << "EOF";
/* 封包资源提取函数 */
static int ${converted}_${name}_extract_resource(struct package *pkg, struct package_resource *pkg_res) {
}

/* 资源保存函数 */
static int ${converted}_${name}_save_resource(struct resource *res, struct package_resource *pkg_res) {
}

/* 封包资源释放函数 */
static void ${converted}_${name}_release_resource(struct package *pkg, struct package_resource *pkg_res) {
}

/* 封包卸载函数 */
static void ${converted}_${name}_release(struct package *pkg, struct package_directory *pkg_dir) {
}

/* 封包处理回调函数集合 */
EOF
	if ($type) {
		$operations .= << "EOF";
static cui_ext_operation ${converted}_${name}_operation = {
	${converted}_${name}_match,			/* match */
	${converted}_${name}_extract_directory,		/* extract_directory */
	${converted}_${name}_parse_resource_info,	/* parse_resource_info */
	${converted}_${name}_extract_resource,		/* extract_resource */
	${converted}_${name}_save_resource,		/* save_resource */
	${converted}_${name}_release_resource,		/* release_resource */
	${converted}_${name}_release			/* release */
};
EOF
	}
	else {
		$operations .= << "EOF";
static cui_ext_operation ${converted}_${name}_operation = {
	${converted}_${name}_match,		/* match */
	NULL,					/* extract_directory */
	NULL,					/* parse_resource_info */
	${converted}_${name}_extract_resource,	/* extract_resource */
	${converted}_${name}_save_resource,	/* save_resource */
	${converted}_${name}_release_resource,	/* release_resource */
	${converted}_${name}_release		/* release */
};
EOF
	}
}

$package =~ s/^\s+|\s+$//g;
$date = POSIX::strftime("%Y-%m-%d %H:%M", localtime);

$cpp = << "EOF";
#include <windows.h>
#include <tchar.h>
#include <crass_types.h>
#include <acui.h>
#include <cui.h>
#include <package.h>
#include <resource.h>
#include <cui_error.h>
#include <stdio.h>

/* 接口数据结构: 表示cui插件的一般信息 */
struct acui_information ${converted}_cui_information = {
	$copyright,	/* copyright */
	$system,	/* system */
	_T("$package"),	/* package */
	$revision,	/* revision */
	$author,	/* author */
	_T("$date"),	/* date */
	$notion,	/* notion */
	ACUI_ATTRIBUTE_LEVEL_$level
};

/* 所有的封包特定的数据结构都要放在这个#pragma段里 */
#pragma pack (1)

#pragma pack ()
$operations
int CALLBACK ${converted}_register_cui(struct cui_register_callback *callback) {
	/* 注册cui插件支持的扩展名、资源放入扩展名、处理回调函数和封包属性 */
$registrations
	return 0;
}
EOF

open(DATA, "> @ARGV[0]/@ARGV[0].cpp");
print DATA $cpp;
close(DATA);
open(DATA, "> @ARGV[0]/@ARGV[0].def");
print DATA $def;
close(DATA);
