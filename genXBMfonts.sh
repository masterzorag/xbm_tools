#!/bin/bash

# file: genXBMfonts.sh
# brief: generate font glyphs and C header file from XBM images
# author: 2015, masterzorag@gmail.com

# This script uses ImageMagick convert to generate images of glyphs

# 1. rebuild $fontDestDir/xbm_font.h
# ./genXBMfonts.sh Razor_1911/ 16 16

# 2. use generated XBM fonts: hardcode in xbm_print
# cp Razor_1911/xbm/xbm_font.h xbm_font.h

# 3. rebuild xbm_tools
# make clean && make


if [ -z $1 ]; then
    echo "$0 FontDir Font_W Font_H"
    echo "$0 Razor_1911 16 16"
    exit
fi

### arguments, keep 16x16 as default

FontDir=$1
Font_W=$2
Font_H=$3

### process argv end here ###

fonts="$fonts $(echo "$FontDir"/*.ttf)"

# ImageMagick supported extension: pnm, png, bmp, xpm, pbm... here we deal with xbm
type=xbm

echo "Found" $(echo "$fonts" | wc -w) "fonts"

# for each font
for i in $fonts
do
    echo "$i"
    fontDestDir="$FontDir/$type"
    mkdir -p "$fontDestDir"

    t=$fontDestDir/temp
    out=$fontDestDir/xbm_font.h
    echo "$t, $out"

    if [ -f "$t" ]
    then
        rm $t 
        rm $out
    fi

    # keep track of array index
    n="0"

    # chars="  ! a b c d e f g h i j k l m n o p q r s t u v w x y z 0 1 2 3 4 5 6 7 8 9 + - / # . , \*"    
    # for c in $chars
    # Better: do a printable range set, start from char ' ' (space): ASCII 32
    # ASCII 127 is NOT printable and will write output.h file in binary form!
    # keep it under 127 for now...
    for c in `seq 32 126`
    do
        #printf "%d\t\x%x\n" $c $c

        # compose decimal representation and label
        d="$(printf %.3d $c)"                    #printf "\x$(printf %x $c)"
        D="$(printf "\x$(printf %x $c)")"

        # 1. build commented label header: array idx, ascii code, glyph
        echo "{ /* $n: ASCII $d [$D] bits */" >> $t

        # Generate (Font_W x Font_H) images, 1bpp of each character in ASCII range 
        # call imagemagick tool to convert bitmap
        convert \
        +antialias \
        -depth 1 -colors 2 \
            -size "$Font_W"x"$Font_H" \
            -background white -fill black \
            -font "$i" \
            -pointsize 17 \
            -density 72 \
            -gravity center \
            label:$D \
            "$fontDestDir/$d.$type" &> /dev/null

        # 2. check to build data bits
        if [ -f "$fontDestDir/$d.$type" ]
        then
            echo "$n: $d.$type [$D]"

            # extra: dump single XBM to console
            ./xbm_dump "$fontDestDir/$d.$type"

            # 3a. strip data bits and push to output
            tail -n+4 "$fontDestDir/$d.$type" | head --bytes -5 >> $t

        else
            echo "$n: $d.$type does not exists!"
            # 3b. build zeroed data bit header for a missing ASCII code
            printf "\t0x00" >> $t

        fi

        # 4. close data bits footer
        printf "\n},\n" >> $t

        # increase array count
        ((n+=1))

    done
    printf "I: range of %d ASCII codes\n" $n

    # 1. build top C header
    printf "#ifndef __XBM_FONT_H__\n" > $out
    printf "#define __XBM_FONT_H__\n" >> $out

    printf "\n/*\n\t%s bits\n" $fonts >> $out
    printf "\tgenerated with genXBMfonts, https://github.com/masterzorag/xbm_tools\n" >> $out
    printf "\t2015, masterzorag@gmail.com\n*/\n\n" >> $out

    printf "#define LOWER_ASCII_CODE %d\n" 32 >> $out
    printf "#define UPPER_ASCII_CODE %d\n" 126 >> $out
    printf "#define FONT_W %d\n" $Font_W >> $out
    printf "#define FONT_H %d\n" $Font_H >> $out
    printf "#define BITS_IN_BYTE %d\n\n" 8 >> $out

    echo "char xbmFont[$n][(FONT_W * FONT_H) / BITS_IN_BYTE] = {" >> $out

    # 2. fix: remove last ','
    head --bytes -2 $t >> $out 

    # 3. build bottom C header: add "};"
    printf "\n};\n" >> $out

    printf "#endif //__XBM_FONT_H__\n" >> $out

    # extra: cleanup from temp
    rm $t

done # for i in fonts


# count and report exported
n=$(ls $fontDestDir/*.$type | wc -l)
printf "I: succesfully parsed %d ASCII codes\nDone\n\n" $n

# inquiry
file $out

# preview outputted C header
head -14 $out
echo "..."
tail -6 $out

# extra: look at exported XBM(s)
viewnior $fontDestDir &> /dev/null

exit
