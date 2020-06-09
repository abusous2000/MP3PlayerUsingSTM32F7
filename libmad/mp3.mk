MP3DIR = ./libmad

MP3SRC  = ${MP3DIR}/bit.c \
          ${MP3DIR}/decoder.c \
          ${MP3DIR}/fixed.c \
          ${MP3DIR}/frame.c \
          ${MP3DIR}/huffman.c \
          ${MP3DIR}/layer12.c \
          ${MP3DIR}/layer3.c \
          ${MP3DIR}/stream.c \
          ${MP3DIR}/synth.c \
          ${MP3DIR}/timer.c \
          ${MP3DIR}/version.c \

# Required include directories
MP3INC = ${MP3DIR}/

ALLCSRC += $(MP3SRC)
ALLINC  += $(MP3INC)
