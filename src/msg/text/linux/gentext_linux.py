import xlrd

def write_to_file(path, content):
    with open(path, 'w') as file:
        file.write(content)

sheets = xlrd.open_workbook('../Messages.xlsx').sheets()

messages = {}
sheet = sheets[2]
for j in range(1, sheet.nrows):
    row = sheet.row_values(j)
    messages[row[0]] = row

# MsgImpl.h
content = """
// generated from gentext.pl Messages.xlsx
#ifndef MsgImplH
#define MsgImplH

#include "tjsMessage.h"

#ifndef TVP_MSG_DECL
	#define TVP_MSG_DECL(name, msg) extern tTJSMessageHolder name;
	#define TVP_MSG_DECL_NULL(name) extern tTJSMessageHolder name;
#endif
//---------------------------------------------------------------------------
// Message Strings
//---------------------------------------------------------------------------
"""
for message in messages.values():
    content += f'TVP_MSG_DECL_NULL({message[0]})\n'
content += "#endif"
tvp_mes_header = 'MsgImpl.h'
write_to_file('../../linux/MsgImpl.h', content)

messages = {}
for sheet in sheets:
    for j in range(1, sheet.nrows):
        row = sheet.row_values(j)
        messages[row[0]] = row

# MsgLoad.cpp
content = """
// generated from gentext.pl Messages.xlsx
#include "tjsCommHead.h"
#include "MsgIntf.h"
#include "tjsError.h"
#include "MsgImpl.h"
#include <libintl.h>
#include <memory>
extern tjs_string getwtext(const char *s);
void TVPLoadMessage() {
    setlocale(LC_ALL, "");
    bindtextdomain("krkrz", "i18n");
    textdomain("krkrz");
"""
for message in messages.values():
    content +=  f'    {message[0]}.AssignMessage(getwtext(gettext("{message[0]}")).c_str());\n'
content += "}"
write_to_file('../../linux/MsgLoad.cpp', content)

# gettext po file
content = """
# Chinese translations for PACKAGE package
# PACKAGE 软件包的简体中文翻译.
# Copyright (C) 2020 THE PACKAGE'S COPYRIGHT HOLDER
# This file is distributed under the same license as the PACKAGE package.
#  <yydcnjjw@gmail.com>, 2020.
#
msgid ""
msgstr ""
"Project-Id-Version: 0.0.1\\n"
"Report-Msgid-Bugs-To: \\n"
"POT-Creation-Date: 2020-01-11 20:33+0800\\n"
"PO-Revision-Date: 2020-01-11 20:44+0800\\n"
"Last-Translator:  <yydcnjjw@gmail.com>\\n"
"Language-Team: Chinese (simplified) <i18n-zh@googlegroups.com>\\n"
"Language: zh_CN\\n"
"MIME-Version: 1.0\\n"
"Content-Type: text/plain; charset=UTF-8\\n"
"Content-Transfer-Encoding: 8bit\\n"

"""
for message in messages.values():
    content += f'msgid "{message[0]}"\n'
    content += f'msgstr "{message[4]}"\n'
    content += '\n'
write_to_file('krkrz_zh.po', content)