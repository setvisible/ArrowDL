# Google Gumbo - A pure-C HTML5 parser
#
# https://github.com/google/gumbo-parser
#

INCLUDEPATH += $$PWD/0.10.1/src

HEADERS += \
    $$PWD/0.10.1/src/attribute.h \
    $$PWD/0.10.1/src/char_ref.h \
    $$PWD/0.10.1/src/error.h \
    $$PWD/0.10.1/src/gumbo.h \
    $$PWD/0.10.1/src/insertion_mode.h \
    $$PWD/0.10.1/src/parser.h \
    $$PWD/0.10.1/src/string_buffer.h \
    $$PWD/0.10.1/src/string_piece.h \
    $$PWD/0.10.1/src/tag_enum.h \
    $$PWD/0.10.1/src/tag_gperf.h \
    $$PWD/0.10.1/src/tag_sizes.h \
    $$PWD/0.10.1/src/tag_strings.h \
    $$PWD/0.10.1/src/tokenizer.h \
    $$PWD/0.10.1/src/tokenizer_states.h \
    $$PWD/0.10.1/src/token_type.h \
    $$PWD/0.10.1/src/utf8.h \
    $$PWD/0.10.1/src/util.h \
    $$PWD/0.10.1/src/vector.h

SOURCES += \
    $$PWD/0.10.1/src/attribute.c \
    $$PWD/0.10.1/src/char_ref.c \
    $$PWD/0.10.1/src/error.c \
    $$PWD/0.10.1/src/parser.c \
    $$PWD/0.10.1/src/string_buffer.c \
    $$PWD/0.10.1/src/string_piece.c \
    $$PWD/0.10.1/src/tag.c \
    $$PWD/0.10.1/src/tokenizer.c \
    $$PWD/0.10.1/src/utf8.c \
    $$PWD/0.10.1/src/util.c \
    $$PWD/0.10.1/src/vector.c

