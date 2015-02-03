/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdio>
#include <cstring>
#include <cassert>

#include <argtable2.h>

#include "str.hpp"
#include "alloc.hpp"
#include "optparse.hpp"

ce_optparse* ce_optparse_new(void)
{
    ce_optparse* optparse = ce_alloc_zero(sizeof(ce_optparse));
    optparse->title = ce_string_new_str("Cursed Earth");
    optparse->brief = ce_string_new();
    optparse->argobjects = ce_vector_new();
    optparse->ctrlobjects = ce_vector_new();

    // some options hard-coded
    ce_optparse_add(optparse, "help", CE_TYPE_BOOL, NULL, false,
        "h", "help", "display this help and exit");
    ce_optparse_add(optparse, "version", CE_TYPE_BOOL, NULL, false,
        "v", "version", "display version information and exit");

    return optparse;
}

void ce_optparse_del(ce_optparse* optparse)
{
    if (NULL != optparse) {
        ce_vector_for_each(optparse->ctrlobjects, (void(*)(void*))ce_object_del);
        ce_vector_for_each(optparse->argobjects, (void(*)(void*))ce_object_del);
        ce_vector_del(optparse->ctrlobjects);
        ce_vector_del(optparse->argobjects);
        ce_string_del(optparse->brief);
        ce_string_del(optparse->title);
        ce_free(optparse, sizeof(ce_optparse));
    }
}

void ce_optparse_set_standard_properties(ce_optparse* optparse,
    int version_major, int version_minor, int version_patch,
    const char* title, const char* brief)
{
    optparse->version_major = version_major;
    optparse->version_minor = version_minor;
    optparse->version_patch = version_patch;
    if (NULL != title) ce_string_assign(optparse->title, title);
    if (NULL != brief) ce_string_assign(optparse->brief, brief);
}

bool ce_optparse_get(ce_optparse* optparse, const char* name, void* value)
{
    const char* tmp;
    for (size_t i = 0; i < optparse->argobjects->count; ++i) {
        ce_object* object = optparse->argobjects->items[i];
        ce_value_get(ce_object_find(object, "name")->value, &tmp);
        if (0 == strcmp(name, tmp)) {
            ce_value_get(ce_object_find(object, "value")->value, value);
            return true;
        }
    }
    return false;
}

void ce_optparse_add(ce_optparse* optparse, const char* name, ce_type type,
                        const void* value, bool required, const char* shortopt,
                        const char* longopt, const char* glossary)
{
    ce_object* object = ce_object_new("unnamed");

    ce_property* propname = ce_property_new("name", CE_TYPE_STRING);
    ce_property* propval = ce_property_new("value", type);
    ce_property* propsopt = ce_property_new("shortopt", CE_TYPE_STRING);
    ce_property* proplopt = ce_property_new("longopt", CE_TYPE_STRING);
    ce_property* propdtype = ce_property_new("datatype", CE_TYPE_STRING);
    ce_property* propmin = ce_property_new("mincount", CE_TYPE_INT);
    ce_property* prophelp = ce_property_new("glossary", CE_TYPE_STRING);

    char datatype[strlen(name) + 1];
    int mincount = required;

    ce_strupr(datatype, name);

    ce_value_set(propname->value, name);
    ce_value_set(propval->value, value);
    ce_value_set(propsopt->value, shortopt);
    ce_value_set(proplopt->value, longopt);
    ce_value_set(propdtype->value, datatype);
    ce_value_set(propmin->value, &mincount);
    ce_value_set(prophelp->value, glossary);

    ce_object_add(object, propname);
    ce_object_add(object, propval);
    ce_object_add(object, propsopt);
    ce_object_add(object, proplopt);
    ce_object_add(object, propdtype);
    ce_object_add(object, propmin);
    ce_object_add(object, prophelp);

    ce_vector_push_back(optparse->argobjects, object);
}

void ce_optparse_add_control(ce_optparse* optparse,
                                const char* name, const char* glossary)
{
    ce_object* object = ce_object_new("unnamed");

    ce_property* propname = ce_property_new("name", CE_TYPE_STRING);
    ce_property* prophelp = ce_property_new("glossary", CE_TYPE_STRING);

    ce_value_set(propname->value, name);
    ce_value_set(prophelp->value, glossary);

    ce_object_add(object, propname);
    ce_object_add(object, prophelp);

    ce_vector_push_back(optparse->ctrlobjects, object);
}

static int ce_optparse_get_props(ce_object* object, const char** shortopt,
    const char** longopt, const char** datatype, const char** glossary)
{
    int mincount;
    ce_value_get(ce_object_find(object, "shortopt")->value, shortopt);
    ce_value_get(ce_object_find(object, "longopt")->value, longopt);
    ce_value_get(ce_object_find(object, "datatype")->value, datatype);
    ce_value_get(ce_object_find(object, "mincount")->value, &mincount);
    ce_value_get(ce_object_find(object, "glossary")->value, glossary);
    return mincount;
}

static void* ce_optparse_create_void(ce_object*)
{
    assert(false);
    return NULL;
}

static void* ce_optparse_create_bool(ce_object* object)
{
    const char *shortopt, *longopt, *datatype, *glossary;
    int mincount = ce_optparse_get_props(object, &shortopt, &longopt, &datatype, &glossary);
    return arg_litn(shortopt, longopt, mincount, 1, glossary);
}

static void* ce_optparse_create_int(ce_object* object)
{
    const char *shortopt, *longopt, *datatype, *glossary;
    int mincount = ce_optparse_get_props(object, &shortopt, &longopt, &datatype, &glossary);
    return arg_intn(shortopt, longopt, datatype, mincount, 1, glossary);
}

static void* ce_optparse_create_float(ce_object* object)
{
    const char *shortopt, *longopt, *datatype, *glossary;
    int mincount = ce_optparse_get_props(object, &shortopt, &longopt, &datatype, &glossary);
    return arg_dbln(shortopt, longopt, datatype, mincount, 1, glossary);
}

static void* ce_optparse_create_string(ce_object* object)
{
    const char *shortopt, *longopt, *datatype, *glossary;
    int mincount = ce_optparse_get_props(object, &shortopt, &longopt, &datatype, &glossary);
    return arg_strn(shortopt, longopt, datatype, mincount, 1, glossary);
}

static void* (*ce_optparse_create_procs[CE_TYPE_COUNT])(ce_object*) = {
    [CE_TYPE_VOID] = ce_optparse_create_void,
    [CE_TYPE_BOOL] = ce_optparse_create_bool,
    [CE_TYPE_INT] = ce_optparse_create_int,
    [CE_TYPE_FLOAT] = ce_optparse_create_float,
    [CE_TYPE_STRING] = ce_optparse_create_string,
};

static void ce_optparse_assign_void(ce_object*, void*)
{
    assert(false);
}

static void ce_optparse_assign_bool(ce_object* object, void* arg)
{
    struct arg_lit* arglit = arg;
    if (0 != arglit->count) {
        ce_value_set(ce_object_find(object, "value")->value, (bool[]){true});
    }
}

static void ce_optparse_assign_int(ce_object* object, void* arg)
{
    struct arg_int* argint = arg;
    if (0 != argint->count) {
        ce_value_set(ce_object_find(object, "value")->value, argint->ival);
    }
}

static void ce_optparse_assign_float(ce_object* object, void* arg)
{
    struct arg_dbl* argdbl = arg;
    if (0 != argdbl->count) {
        float val = argdbl->dval[0];
        ce_value_set(ce_object_find(object, "value")->value, &val);
    }
}

static void ce_optparse_assign_string(ce_object* object, void* arg)
{
    struct arg_str* argstr = arg;
    if (0 != argstr->count) {
        ce_value_set(ce_object_find(object, "value")->value, argstr->sval[0]);
    }
}

static void (*ce_optparse_assign_procs[CE_TYPE_COUNT])(ce_object*, void*) = {
    [CE_TYPE_VOID] = ce_optparse_assign_void,
    [CE_TYPE_BOOL] = ce_optparse_assign_bool,
    [CE_TYPE_INT] = ce_optparse_assign_int,
    [CE_TYPE_FLOAT] = ce_optparse_assign_float,
    [CE_TYPE_STRING] = ce_optparse_assign_string,
};

static void ce_optparse_usage(ce_optparse* optparse,
                            void* argtable[], const char* progname)
{
    fprintf(stderr, "Cursed Earth is an open source, "
        "cross-platform port of Evil Islands\n"
        "Copyright (C) 2009-2010 Yanis Kurganov\n\n");

    if (!ce_string_empty(optparse->brief)) {
        fprintf(stderr, "%s\n\n", optparse->brief->str);
    }

    fprintf(stderr, "usage: %s", progname);
    arg_print_syntax(stderr, argtable, "\n");
    arg_print_glossary_gnu(stderr, argtable);

    if (!ce_vector_empty(optparse->ctrlobjects)) {
        const char *name, *glossary;
        void* ctrltable[optparse->ctrlobjects->count + 1];
        ctrltable[optparse->ctrlobjects->count] = arg_end(0);

        for (size_t i = 0; i < optparse->ctrlobjects->count; ++i) {
            ce_object* object = optparse->ctrlobjects->items[i];
            ce_value_get(ce_object_find(object, "name")->value, &name);
            ce_value_get(ce_object_find(object, "glossary")->value, &glossary);
            ctrltable[i] = arg_rem(name, glossary);
        }

        fprintf(stderr, "controls:\n");
        arg_print_glossary_gnu(stderr, ctrltable);

        arg_freetable(ctrltable, sizeof(ctrltable) / sizeof(ctrltable[0]));
    }
}

bool ce_optparse_parse(ce_optparse* optparse, int argc, char* argv[])
{
    void* argtable[optparse->argobjects->count + 1];
    argtable[optparse->argobjects->count] = arg_end(3);

    for (size_t i = 0; i < optparse->argobjects->count; ++i) {
        ce_object* object = optparse->argobjects->items[i];
        ce_type type = ce_object_find(object, "value")->value->type;
        argtable[i] = (*ce_optparse_create_procs[type])(object);
    }

    int error_count = arg_parse(argc, argv, argtable);

    for (size_t i = 0; i < optparse->argobjects->count; ++i) {
        ce_object* object = optparse->argobjects->items[i];
        ce_type type = ce_object_find(object, "value")->value->type;
        (*ce_optparse_assign_procs[type])(object, argtable[i]);
    }

    bool help, version;
    ce_optparse_get(optparse, "help", &help);
    ce_optparse_get(optparse, "version", &version);

    // special case: help/version takes precedence over error reporting
    if (help) {
        ce_optparse_usage(optparse, argtable, argv[0]);
    } else if (version) {
        fprintf(stderr, "%d.%d.%d\n", optparse->version_major,
            optparse->version_minor, optparse->version_patch);
    } else if (0 != error_count) {
        ce_optparse_usage(optparse, argtable, argv[0]);
        arg_print_errors(stderr, argtable[optparse->argobjects->count], argv[0]);
    }

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

    return !help && !version && 0 == error_count;
}
