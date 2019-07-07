# Google Gumbo - A pure-C HTML5 parser
#
# https://github.com/google/gumbo-parser
#

INCLUDEPATH += $$PWD/gumbo-0.10.1/src

HEADERS += \
    $$PWD/gumbo-0.10.1/src/attribute.h \
    $$PWD/gumbo-0.10.1/src/char_ref.h \
    $$PWD/gumbo-0.10.1/src/error.h \
    $$PWD/gumbo-0.10.1/src/gumbo.h \
    $$PWD/gumbo-0.10.1/src/insertion_mode.h \
    $$PWD/gumbo-0.10.1/src/parser.h \
    $$PWD/gumbo-0.10.1/src/string_buffer.h \
    $$PWD/gumbo-0.10.1/src/string_piece.h \
    $$PWD/gumbo-0.10.1/src/tag_enum.h \
    $$PWD/gumbo-0.10.1/src/tag_gperf.h \
    $$PWD/gumbo-0.10.1/src/tag_sizes.h \
    $$PWD/gumbo-0.10.1/src/tag_strings.h \
    $$PWD/gumbo-0.10.1/src/tokenizer.h \
    $$PWD/gumbo-0.10.1/src/tokenizer_states.h \
    $$PWD/gumbo-0.10.1/src/token_type.h \
    $$PWD/gumbo-0.10.1/src/utf8.h \
    $$PWD/gumbo-0.10.1/src/util.h \
    $$PWD/gumbo-0.10.1/src/vector.h

SOURCES += \
    $$PWD/gumbo-0.10.1/src/attribute.c \
    $$PWD/gumbo-0.10.1/src/char_ref.c \
    $$PWD/gumbo-0.10.1/src/error.c \
    $$PWD/gumbo-0.10.1/src/parser.c \
    $$PWD/gumbo-0.10.1/src/string_buffer.c \
    $$PWD/gumbo-0.10.1/src/string_piece.c \
    $$PWD/gumbo-0.10.1/src/tag.c \
    $$PWD/gumbo-0.10.1/src/tokenizer.c \
    $$PWD/gumbo-0.10.1/src/utf8.c \
    $$PWD/gumbo-0.10.1/src/util.c \
    $$PWD/gumbo-0.10.1/src/vector.c

