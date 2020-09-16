#!/usr/bin/python3

import sys
import os
import subprocess

PO_OUTPUT_DIRNAME = "po"
TRANSL_SRC_FILE_PATH = "../../src/utils/translation.hpp"
DOMAIN_NAME = "redemption"
MO_CATEGORY_DIRNAME = "LC_MESSAGES"



def check_po_dir_path(po_dir_path) :
    if not os.path.exists(po_dir_path) :
        raise OSError("missing '" + po_dir_path + "' directory")
    elif not os.path.isdir(po_dir_path) :
        raise OSError("cannot use '" + po_dir_path + "' as directory")

def check_msg_catalog_template_path(po_dir_path) :
    msg_catalog_template_file = po_dir_path + '/' + DOMAIN_NAME + ".pot"
    
    if not os.path.exists(msg_catalog_template_file) :
        raise OSError("missing '"
                      + msg_catalog_template_file
                      + "' file")
    elif not os.path.isfile(msg_catalog_template_file) :
        raise OSError("cannot use '"
                      + msg_catalog_template_file
                      + "' as file")

def check_target_locale_dir_path(target_locale_dir_path) :
    if not os.path.exists(target_locale_dir_path) :
        raise OSError("missing '"
                      + target_locale_dir_path
                      + "' directory")
    elif not os.path.isdir(target_locale_dir_path) :
        raise OSError("cannot use '"
                      + target_locale_dir_path
                      + "' as directory")

def check_transl_msg_catalog_path(target_locale_dir_path) :
    transl_msg_catalog_file = (target_locale_dir_path
                               + '/'
                               + DOMAIN_NAME
                               + ".po")
    
    if not os.path.exists(transl_msg_catalog_file) :
        raise OSError("missing '"
                      + transl_msg_catalog_file
                      + "' file")
    elif not os.path.isfile(transl_msg_catalog_file) :
        raise OSError("cannot use '"
                      + transl_msg_catalog_file 
                      + "' as file")



def gen_msg_catalog_template(po_path) :
    po_dir_path = po_path + '/' + PO_OUTPUT_DIRNAME
    
    if not os.path.exists(po_dir_path) :
        os.mkdir(po_dir_path)
        print("'" + po_dir_path + "' directory has been generated")
    else :
        check_po_dir_path(po_dir_path)

    list_xgettext_cmd = [
        "xgettext",
        "--keyword=TR_KV:2",
        "--keyword=TR_KV_FMT:2",
        "--keyword=TR_PROTECTED_KV:2",
        "--c++",
        "--sort-by-file",
        "--output="
        + po_dir_path + '/' + DOMAIN_NAME + ".pot",
        TRANSL_SRC_FILE_PATH]

    if subprocess.call(list_xgettext_cmd,
                       shell=False,
                       stdout=subprocess.DEVNULL,
                       stderr=subprocess.STDOUT) != 0 :
        raise OSError("error during message "
                      "catalog template generation "
                      "(maybe you executed script in the wrong directory)")

    print("'"
          + po_dir_path
          + '/'
          + DOMAIN_NAME
          + ".pot' file has been generated")

def gen_transl_msg_catalog(target_locale, po_path) :
    po_dir_path = po_path + '/' + PO_OUTPUT_DIRNAME
    
    check_po_dir_path(po_dir_path)
    check_msg_catalog_template_path(po_dir_path)
    
    target_locale_dir_path = po_dir_path + '/' + target_locale
    
    if not os.path.exists(target_locale_dir_path) :
        os.mkdir(target_locale_dir_path)
        print("'" + target_locale_dir_path + "' directory has been generated")
    else :
        check_target_locale_dir_path(target_locale_dir_path)

    list_msginit_cmd = [
        "msginit",
        "--no-translator",
        "--no-wrap",
        "--locale=" + target_locale,
        "--input=" + po_dir_path + '/' + DOMAIN_NAME + ".pot",
        "--output=" + target_locale_dir_path + '/' + DOMAIN_NAME + ".po"]

    if subprocess.call(list_msginit_cmd,
                       shell=False,
                       stdout=subprocess.DEVNULL,
                       stderr=subprocess.STDOUT) != 0 :
        raise OSError("error during '"
                      + target_locale
                      + "' message catalog generation")

    print("'"
          + target_locale_dir_path
          + '/'
          + DOMAIN_NAME
          + ".po' file has been generated")

def gen_modified_transl_msg_catalog_from_tpl(target_locale, po_path) :
    po_dir_path = po_path + '/' + PO_OUTPUT_DIRNAME
    
    check_po_dir_path(po_dir_path)
    check_msg_catalog_template_path(po_dir_path)

    target_locale_dir_path = po_dir_path + '/' + target_locale

    check_target_locale_dir_path(target_locale_dir_path)
    check_transl_msg_catalog_path(target_locale_dir_path)

    list_msgmerge_cmd = [
        "msgmerge",
        "--update",
        "--force-po",
        "--no-wrap",
        target_locale_dir_path + '/' + DOMAIN_NAME + ".po",
        po_dir_path + '/' + DOMAIN_NAME + ".pot"]

    if subprocess.call(list_msgmerge_cmd,
                       shell=False,
                       stdout=subprocess.DEVNULL,
                       stderr=subprocess.STDOUT) != 0 :
        raise OSError("error during '"
                      + target_locale
                      + "' message catalog modification "
                      "(maybe in your .po file, charset is wrong)")
    
    print("'"
          + target_locale_dir_path
          + '/'
          + DOMAIN_NAME
          + ".po' file has been modified")

def gen_compiled_msg_catalog(target_locale, po_path, mo_path) :
    po_dir_path = po_path + '/' + PO_OUTPUT_DIRNAME
    
    check_po_dir_path(po_dir_path)

    po_target_locale_dir_path = po_dir_path + '/' + target_locale

    check_target_locale_dir_path(po_target_locale_dir_path)
    check_transl_msg_catalog_path(po_target_locale_dir_path)

    if not os.path.exists(mo_path) :
        os.mkdir(mo_path)
        print("'" + mo_path + "' directory has been generated")
    elif not os.path.isdir(mo_path) :
        raise OSError("cannot use '" + mo_path + "' as directory")

    mo_target_locale_dir_path = mo_path + '/' + target_locale
    
    if not os.path.exists(mo_target_locale_dir_path) :
        os.mkdir(mo_target_locale_dir_path)
        print("'"
              + mo_target_locale_dir_path
              + "' directory has been generated")
    elif not os.path.isdir(mo_target_locale_dir_path) :
        raise OSError("cannot use '"
                      + mo_target_locale_dir_path
                      + "' as directory")

    mo_category_dir_path = (mo_target_locale_dir_path
                            + '/'
                            + MO_CATEGORY_DIRNAME)

    if not os.path.exists(mo_category_dir_path) :
        os.mkdir(mo_category_dir_path)
        print("'" + mo_category_dir_path + "' directory has been generated")
    elif not os.path.isdir(mo_category_dir_path) :
        raise OSError("cannot use '" + mo_category_dir_path + "' as directory")

    list_msgfmt_cmd = [
        "msgfmt",
        "--output-file="
        + mo_category_dir_path + '/' + DOMAIN_NAME + ".mo",
        po_target_locale_dir_path + '/' + DOMAIN_NAME + ".po"]
    
    if subprocess.call(list_msgfmt_cmd,
                       shell=False,
                       stdout=subprocess.DEVNULL,
                       stderr=subprocess.STDOUT) != 0 :
        raise OSError("error during compiled message catalog generation in '"
                      + target_locale
                      + "' locale")

    print("'"
          + mo_category_dir_path
          + '/'
          + DOMAIN_NAME
          + ".mo' file has been generated")

def gen_all_modified_transl_msg_catalogs_from_tpl(po_path) :
    po_dir_path = po_path + '/' + PO_OUTPUT_DIRNAME
    
    check_po_dir_path(po_dir_path)
    
    target_locale_dir_name_list = ([target_locale_dir_name
        for target_locale_dir_name in os.listdir(po_dir_path)
        if os.path.isdir(po_dir_path + '/' + target_locale_dir_name)])
    
    if not target_locale_dir_name_list :
        raise RuntimeError("no locale directory in '"
                           + po_dir_path
                           + "' directory")
    
    for target_locale_dir_name in target_locale_dir_name_list :
        gen_modified_transl_msg_catalog_from_tpl(target_locale_dir_name,
                                                 po_path)
    
def gen_all_compiled_msg_catalogs(po_path, mo_path) :
    po_path_dir = po_path + '/' + PO_OUTPUT_DIRNAME
    
    check_po_dir_path(po_path_dir)

    target_locale_dir_name_list = ([target_locale_dir_name
        for target_locale_dir_name in os.listdir(po_path_dir)
        if os.path.isdir(po_path_dir + '/' + target_locale_dir_name)])
    
    if not target_locale_dir_name_list :
        raise RuntimeError("no locale directory in '"
                           + po_path_dir
                           + "' directory")
    
    for target_locale_dir_name in target_locale_dir_name_list :
        gen_compiled_msg_catalog(target_locale_dir_name, po_path, mo_path)

def execute_generation(args) :    
    if args.extract_source :
        gen_msg_catalog_template(args.po_path)
    elif args.init_catalog :
        gen_transl_msg_catalog(args.locale, args.po_path)
    elif args.merge_catalog_with_template :
        gen_modified_transl_msg_catalog_from_tpl(args.locale, args.po_path)
    elif args.merge_all_catalogs_with_template :
        gen_all_modified_transl_msg_catalogs_from_tpl(args.po_path)
    elif args.compile_catalog :
        gen_compiled_msg_catalog(args.locale, args.po_path, args.mo_path)
    elif args.compile_all_catalogs :
        gen_all_compiled_msg_catalogs(args.po_path, args.mo_path)
    else :
        assert False

def parse_arguments() :
    import argparse
    
    usage_example_desc = ("example usage with 'en' locale: \n"
                          + sys.argv[0] + " -e -p .\n"
                          + sys.argv[0] + " -i -l en -p .\n"
                          + sys.argv[0] + " -m -l en -p .\n"
                          + sys.argv[0] + " -M -p .\n"
                          + sys.argv[0] + " -c -l en -p . -b .\n"
                          + sys.argv[0] + " -C -p . -b .")
    parser = argparse.ArgumentParser(
        description=usage_example_desc,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        add_help=False)
    mandatory_grp = (parser.add_argument_group(
        "exclusive mandatory arguments"
    ).add_mutually_exclusive_group(required=True))

    mandatory_grp.add_argument("-e",
                               "--extract-source",
                               action="store_true",
                               help=("extract gettext strings from "
                                     "translation source file of "
                                     "Redemption project by generating .pot "
                                     "template file "
                                     "(to use with -p option)"))
    mandatory_grp.add_argument("-i",
                               "--init-catalog",
                               action="store_true",
                               help=("initialize message catalog by "
                                     "generating .po file (if .po file "
                                     "already exists, use rather -m option " 
                                     "for avoid overwritten) "
                                     "[to use with -l, -p options]"))
    mandatory_grp.add_argument("-m",
                               "--merge-catalog-with-template",
                               action="store_true",
                               help=("merge message catalog and "
                                     "template by keeping "
                                     "the .po existing file content "
                                     "[to use with -l, -p options]"))
    mandatory_grp.add_argument("-M",
                               "--merge-all-catalogs-with-template",
                               action="store_true",
                               help=("merge all message catalogs and template "
                                     "by keeping the .po existing "
                                     "files contents "
                                     "[to use with -p option]"))
    mandatory_grp.add_argument("-c",
                               "--compile-catalog",
                               action="store_true",
                               help=("compile message catalog by "
                                     "generating .mo file "
                                     "[to use with -l, -p, -b options]"))
    mandatory_grp.add_argument("-C",
                               "--compile-all-catalogs",
                               action="store_true",
                               help=("compile all message catalogs by "
                                     "generating .mo files "
                                     "[to use with -p, -b options]"))
    
    optional_grp = parser.add_argument_group("optional arguments")

    optional_grp.add_argument("-h",
                              "--help",
                              action="help",
                              help=("show this help message and exit"))
    optional_grp.add_argument("-p",
                              "--po-path",
                              help=("give a path for po base directory name"),
                              metavar="PATH")
    optional_grp.add_argument("-b",
                              "--mo-path",
                              help=("give a path for mo base directory name"),
                              metavar="PATH")
    optional_grp.add_argument("-l",
                              "--locale",
                              help=("give a target locale"),
                              metavar="LOCALE")

    args = parser.parse_args()

    if args.extract_source :
        if not args.po_path :
            parser.error("need -p/--po-path option")
    elif args.init_catalog :
        if not args.locale or not args.po_path :
            parser.error("need -l/--locale, -p/--po-path options")
    elif args.merge_catalog_with_template :
        if not args.locale or not args.po_path :
            parser.error("need -l/--locale, -p/--po-path options")
    elif args.merge_all_catalogs_with_template :
        if not args.po_path :
            parser.error("need -p/--po-path option")
    elif args.compile_catalog :
        if not args.po_path or not args.mo_path or not args.locale :
            parser.error("need -p/--po-path, -b/--mo-path, -l/--locale "
                         "options")
    elif args.compile_all_catalogs :
        if not args.po_path or not args.mo_path :
            parser.error("need -p/--po-path, -b/--mo-path options")
    else :
        assert False

    return args




if __name__ == "__main__" :
    args = parse_arguments()
    
    try :
        execute_generation(args)
    except :
        print(sys.exc_info()[1])
        sys.exit(1)
    else :
        sys.exit(0)
