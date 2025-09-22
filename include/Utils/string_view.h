#ifndef STRING_VIEW
#define STRING_VIEW
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

typedef struct String_View String_View;

struct String_View {
    uint64_t len;
    const char* data;
};

#define STR(s) ((String_View) { .len = strlen(s), .data = s })
#define Str_Fmt(s) (int)s.len, s.data

String_View sv_trim_left(String_View s)
{
    size_t i = 0;
    while (i < s.len && isspace(s.data[i])) {
        i++;
    }
    return (String_View) { .len = s.len - i, .data = s.data + i };
}

String_View sv_trim_right(String_View s)
{
    size_t i = s.len;
    while (i > 0 && isspace(s.data[i - 1])) {
        i--;
    }
    return (String_View) { .len = i, .data = s.data };
}

String_View sv_trim(String_View s)
{
    return sv_trim_right(sv_trim_left(s));
}

bool sv_compare(String_View a, String_View b)
{
    if (a.len != b.len) {
        return false;
    } else {
        return memcmp(a.data, b.data, a.len) == 0;
    }
}

bool sv_index_of(String_View str, char c, size_t* index)
{
    size_t i = 0;
    while (i < str.len && str.data[i] != c) {
        i += 1;
    }

    if (i < str.len) {
        if (index)
            *index = i;
        return true;
    }
    return false;
}

String_View sv_split_by_len(String_View* str, size_t n)
{
    if (n > str->len) {
        n = str->len;
    }

    String_View result = {
        .data = str->data,
        .len  = n,
    };

    str->data += n;
    str->len -= n;

    return result;
}

String_View sv_split_by_len_reversed(String_View* str, size_t n)
{
    if (n > str->len) {
        n = str->len;
    }

    String_View result = {
        .data = str->data + str->len - n,
        .len  = n
    };

    str->len -= n;

    return result;
}

String_View sv_split_by_delim(String_View* s, char c)
{
    size_t i = 0;
    while (i < s->len && s->data[i] != c) {
        i++;
    }

    String_View res = sv_split_by_len(s, i);

    if (s->len > 0 && s->data[0] == c) {     // discard the delim
        s->len -= 1;
        s->data += 1;
    }

    return res;
}

String_View sv_split_by_condition(String_View* str, bool (*predicate)(char x))
{
    size_t i = 0;
    while (i < str->len && predicate(str->data[i])) {
        i += 1;
    }

    return sv_split_by_len(str, i);
}

bool sv_starts_with(String_View str, String_View prefix)
{
    if (prefix.len <= str.len) {
        for (size_t i = 0; i < prefix.len; ++i) {
            if (prefix.data[i] != str.data[i]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool sv_ends_with(String_View str, String_View expected)
{
    if (expected.len <= str.len) {
        String_View actual = {
            .data = str.data + str.len - expected.len,
            .len  = expected.len
        };

        return sv_compare(expected, actual);
    }

    return false;
}

#endif