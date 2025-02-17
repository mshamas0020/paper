#!/bin/bash
# Usage: ttf_embed.sh input_ttf

set -e

fontname="$(basename $1)"
fontname="${fontname//[^[:alnum:]]/_}"
fontname="${fontname,,}"

# pip install fonttools
pyftsubset "$1" --output-file="./temp.tff" --unicodes=U+0020-007E
xxd -i -n "$fontname" "./temp.tff" "${fontname}.h"
rm "./temp.tff"