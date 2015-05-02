#!/bin/bash

# file: genXBMfonts.sh
# brief: generate font glyphs and C header file from XBM images
# author: 2015, masterzorag@gmail.com

# This script uses ImageMagick convert to generate images of glyphs

FontDir="Razor_1911"

#fonts=""
fonts="$fonts $(echo "$FontDir"/*.ttf)"

# ImageMagick supported extension: pnm, png, bmp, xpm, pbm... here we deal with xbm
type=xbm

echo "Found" $(echo "$fonts" | wc -w) "fonts"

# for each font
for i in $fonts
do
    fontName=razors.ttf
    echo "$fontName"
    
    fontDestDir="$fontName/$type"    
    mkdir -p "$fontDestDir"
    
    t=$fontDestDir/temp
    echo "$t"
    
	if [ -f "$t" ]
	then
		rm $t 
		rm $fontDestDir/$fontName.h
	fi
	
	# keep track of array index
	n="0"

	# chars="  ! a b c d e f g h i j k l m n o p q r s t u v w x y z 0 1 2 3 4 5 6 7 8 9 + - / # . , \*"	
	# for c in $chars
	# Better: do a printable range set, start from char ' ' (space): ASCII 32
	# ASCII 127 is NOT printable and will write $fontName.h file in binary form!
	# keep it under 127 for now...
    for c in `seq 32 126`
    do
		#printf "%d\t\x%x\n" $c $c
		
		# compose decimal representation and label
		d="$(printf %.3d $c)"					#printf "\x$(printf %x $c)"
		D="$(printf "\x$(printf %x $c)")"
		
		# 1. build commented label header: array idx, ascii code, glyph
		echo "{ /* $n: ASCII $d [$D] bits */" >> $t
    
    	# Generate 16x16 images, 1bpp of each character in ASCII range 
    	# call imagemagick tool to convert bitmap(s)
    	convert \
		+antialias \
		-depth 1 -colors 2 \
            -size 16x16 \
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
	printf "/*\n\t%s bits\n" $fontName > $fontDestDir/$fontName.h
	printf "\tgenerated with genXBMfonts, https://github.com/masterzorag/xbm_tools\n" >> $fontDestDir/$fontName.h
	printf "\t2015, masterzorag@gmail.com\n*/\n\n" >> $fontDestDir/$fontName.h
	
	printf "#define LOWER_ASCII_CODE %d\n" 32 >> $fontDestDir/$fontName.h
	printf "#define UPPER_ASCII_CODE %d\n" 126 >> $fontDestDir/$fontName.h
	printf "#define FONT_W %d\n" 16 >> $fontDestDir/$fontName.h
	printf "#define FONT_H %d\n" 16 >> $fontDestDir/$fontName.h
	printf "#define BITS_IN_BYTE %d\n\n" 8 >> $fontDestDir/$fontName.h
	
	echo "char xbmFont[$n][32] = {" >> $fontDestDir/$fontName.h
    
    # 2. fix: remove last ','
	head --bytes -2 $t >> $fontDestDir/$fontName.h 
    
    # 3. build bottom C header: add "};"
	printf "\n};\n" >> $fontDestDir/$fontName.h

	# extra: cleanup from temp
	rm $t

done # for i in fonts


# count and report exported
n=$(ls $fontDestDir/*.$type | wc -l)
printf "I: succesfully parsed %d ASCII codes\nDone\n\n" $n

# inquiry, preview outputted C header
file $fontDestDir/$fontName.h

head -10 $fontDestDir/$fontName.h
echo "..."
tail -6 $fontDestDir/$fontName.h

# extra: look at exported XBM(s)
viewnior $fontDestDir &> /dev/null

exit

# 1. rebuild $fontDestDir/$fontName.h
# ./genXBMfonts.sh

# 2. use generated XBM fonts: hardcode in xbm_print
# cp razors.ttf/xbm/razors.ttf.h xbm_font.h

# 3. rebuild xbm_tools
# make clean && make
