/* C++ code produced by gperf version 3.0.3 */
/* Command-line: gperf --initializer-suffix=',""' OXML_LangToScriptConverter.gperf  */
/* Computed positions: -k'1-2' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "OXML_LangToScriptConverter.gperf"

/* AbiSource
 *
 * Copyright (C) 2007 Philippe Milot <PhilMilot@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

/* DO NOT EDIT, edit the .gperf file isntead ! */

#line 29 "OXML_LangToScriptConverter.gperf"
struct OXML_LangScriptAsso {
       const char *lang;
       const char *script;
};

#define TOTAL_KEYWORDS 185
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 2
#define MIN_HASH_VALUE 6
#define MAX_HASH_VALUE 501
/* maximum key range = 496, duplicates = 0 */

class OXML_LangToScriptConverter
{
private:
  static inline unsigned int hash (const char *str, unsigned int len);
public:
  static struct OXML_LangScriptAsso *in_word_set (const char *str, unsigned int len);
};

inline unsigned int
OXML_LangToScriptConverter::hash (register const char *str, register unsigned int len)
{
  static unsigned short asso_values[] =
    {
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      195,  95,  39, 255, 240, 254,  89,  19,  80,  14,
      190,  94,   4, 165, 159, 214,  65,  99,   5,   0,
      100,  38,  55,  23,  15, 245, 170,  13,  28,   4,
       35, 502,  20, 185, 210, 179, 235, 119,  38, 502,
       44, 239, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
      502, 502, 502, 502, 502
    };
  return len + asso_values[(unsigned char)str[1]+19] + asso_values[(unsigned char)str[0]+3];
}

struct OXML_LangScriptAsso *
OXML_LangToScriptConverter::in_word_set (register const char *str, register unsigned int len)
{
  static struct OXML_LangScriptAsso wordlist[] =
    {
      {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},
#line 197 "OXML_LangToScriptConverter.gperf"
      {"tn", "Latn"},
#line 173 "OXML_LangToScriptConverter.gperf"
      {"sd", "Arab"},
      {"",""}, {"",""},
#line 138 "OXML_LangToScriptConverter.gperf"
      {"mn", "Mong"},
#line 181 "OXML_LangToScriptConverter.gperf"
      {"sn", "Latn"},
#line 172 "OXML_LangToScriptConverter.gperf"
      {"sc", "Latn"},
      {"",""}, {"",""},
#line 196 "OXML_LangToScriptConverter.gperf"
      {"tl", "Latn"},
      {"",""},
#line 194 "OXML_LangToScriptConverter.gperf"
      {"ti", "Ethi"},
      {"",""},
#line 137 "OXML_LangToScriptConverter.gperf"
      {"ml", "Mlym"},
#line 179 "OXML_LangToScriptConverter.gperf"
      {"sl", "Latn"},
#line 135 "OXML_LangToScriptConverter.gperf"
      {"mi", "Latn"},
#line 177 "OXML_LangToScriptConverter.gperf"
      {"si", "Sinh"},
      {"",""}, {"",""},
#line 193 "OXML_LangToScriptConverter.gperf"
      {"th", "Thai"},
      {"",""},
#line 183 "OXML_LangToScriptConverter.gperf"
      {"sq", "Latn"},
      {"",""},
#line 134 "OXML_LangToScriptConverter.gperf"
      {"mh", "Latn"},
#line 176 "OXML_LangToScriptConverter.gperf"
      {"sh", "Cyrl"},
      {"",""},
#line 214 "OXML_LangToScriptConverter.gperf"
      {"yi", "Hebr"},
      {"",""}, {"",""},
#line 180 "OXML_LangToScriptConverter.gperf"
      {"sm", "Latn"},
#line 90 "OXML_LangToScriptConverter.gperf"
      {"hi", "Deva"},
#line 198 "OXML_LangToScriptConverter.gperf"
      {"to", "Latn"},
      {"",""}, {"",""},
#line 202 "OXML_LangToScriptConverter.gperf"
      {"tw", "Latn"},
#line 139 "OXML_LangToScriptConverter.gperf"
      {"mo", "Cyrl"},
#line 182 "OXML_LangToScriptConverter.gperf"
      {"so", "Latn"},
      {"",""}, {"",""},
#line 189 "OXML_LangToScriptConverter.gperf"
      {"sw", "Latn"},
#line 203 "OXML_LangToScriptConverter.gperf"
      {"ty", "Latn"},
      {"",""},
#line 213 "OXML_LangToScriptConverter.gperf"
      {"xh", "Latn"},
      {"",""},
#line 143 "OXML_LangToScriptConverter.gperf"
      {"my", "Mymr"},
      {"",""},
#line 215 "OXML_LangToScriptConverter.gperf"
      {"yo", "Latn"},
      {"",""}, {"",""},
#line 209 "OXML_LangToScriptConverter.gperf"
      {"vi", "Latn"},
#line 91 "OXML_LangToScriptConverter.gperf"
      {"ho", "Latn"},
#line 192 "OXML_LangToScriptConverter.gperf"
      {"tg", "Arab"},
      {"",""}, {"",""}, {"",""},
#line 133 "OXML_LangToScriptConverter.gperf"
      {"mg", "Latn"},
#line 175 "OXML_LangToScriptConverter.gperf"
      {"sg", "Latn"},
      {"",""}, {"",""},
#line 95 "OXML_LangToScriptConverter.gperf"
      {"hy", "Armn"},
      {"",""},
#line 190 "OXML_LangToScriptConverter.gperf"
      {"ta", "Taml"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 171 "OXML_LangToScriptConverter.gperf"
      {"sa", "Deva"},
      {"",""}, {"",""},
#line 210 "OXML_LangToScriptConverter.gperf"
      {"vo", "Latn"},
#line 58 "OXML_LangToScriptConverter.gperf"
      {"co", "Latn"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 107 "OXML_LangToScriptConverter.gperf"
      {"ja", "Jpan"},
#line 98 "OXML_LangToScriptConverter.gperf"
      {"id", "Latn"},
      {"",""}, {"",""},
#line 63 "OXML_LangToScriptConverter.gperf"
      {"cy", "Latn"},
#line 88 "OXML_LangToScriptConverter.gperf"
      {"ha", "Latn"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 83 "OXML_LangToScriptConverter.gperf"
      {"gd", "Latn"},
#line 212 "OXML_LangToScriptConverter.gperf"
      {"wo", "Latn"},
      {"",""}, {"",""},
#line 85 "OXML_LangToScriptConverter.gperf"
      {"gn", "Latn"},
      {"",""},
#line 101 "OXML_LangToScriptConverter.gperf"
      {"ii", "Yiii"},
      {"",""}, {"",""},
#line 128 "OXML_LangToScriptConverter.gperf"
      {"ln", "Latn"},
#line 52 "OXML_LangToScriptConverter.gperf"
      {"bn", "Beng"},
#line 191 "OXML_LangToScriptConverter.gperf"
      {"te", "Telu"},
      {"",""},
#line 84 "OXML_LangToScriptConverter.gperf"
      {"gl", "Latn"},
#line 167 "OXML_LangToScriptConverter.gperf"
      {"rn", "Latn"},
#line 56 "OXML_LangToScriptConverter.gperf"
      {"ca", "Latn"},
#line 174 "OXML_LangToScriptConverter.gperf"
      {"se", "Latn"},
      {"",""}, {"",""}, {"",""},
#line 127 "OXML_LangToScriptConverter.gperf"
      {"li", "Latn"},
#line 50 "OXML_LangToScriptConverter.gperf"
      {"bi", "Latn"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 103 "OXML_LangToScriptConverter.gperf"
      {"io", "Latn"},
      {"",""}, {"",""},
#line 49 "OXML_LangToScriptConverter.gperf"
      {"bh", "Deva"},
#line 89 "OXML_LangToScriptConverter.gperf"
      {"he", "Hebr"},
#line 211 "OXML_LangToScriptConverter.gperf"
      {"wa", "Latn"},
      {"",""}, {"",""},
#line 51 "OXML_LangToScriptConverter.gperf"
      {"bm", "Latn"},
#line 188 "OXML_LangToScriptConverter.gperf"
      {"sv", "Latn"},
      {"",""}, {"",""},
#line 166 "OXML_LangToScriptConverter.gperf"
      {"rm", "Latn"},
      {"",""},
#line 129 "OXML_LangToScriptConverter.gperf"
      {"lo", "Laoo"},
#line 53 "OXML_LangToScriptConverter.gperf"
      {"bo", "Tibt"},
      {"",""}, {"",""},
#line 108 "OXML_LangToScriptConverter.gperf"
      {"jv", "Java"},
#line 168 "OXML_LangToScriptConverter.gperf"
      {"ro", "Latn"},
#line 100 "OXML_LangToScriptConverter.gperf"
      {"ig", "Latn"},
      {"",""},
#line 170 "OXML_LangToScriptConverter.gperf"
      {"rw", "Latn"},
#line 208 "OXML_LangToScriptConverter.gperf"
      {"ve", "Latn"},
#line 57 "OXML_LangToScriptConverter.gperf"
      {"ce", "Cyrl"},
      {"",""}, {"",""}, {"",""}, {"",""}, {"",""},
#line 97 "OXML_LangToScriptConverter.gperf"
      {"ia", "Latn"},
      {"",""}, {"",""}, {"",""},
#line 126 "OXML_LangToScriptConverter.gperf"
      {"lg", "Latn"},
#line 48 "OXML_LangToScriptConverter.gperf"
      {"bg", "Cyrl"},
      {"",""}, {"",""}, {"",""},
#line 82 "OXML_LangToScriptConverter.gperf"
      {"ga", "Latn"},
#line 204 "OXML_LangToScriptConverter.gperf"
      {"ug", "Arab"},
      {"",""}, {"",""},
#line 62 "OXML_LangToScriptConverter.gperf"
      {"cv", "Cyrl"},
#line 124 "OXML_LangToScriptConverter.gperf"
      {"la", "Latn"},
#line 46 "OXML_LangToScriptConverter.gperf"
      {"ba", "Cyrl"},
      {"",""}, {"",""}, {"",""},
#line 155 "OXML_LangToScriptConverter.gperf"
      {"oc", "Latn"},
#line 146 "OXML_LangToScriptConverter.gperf"
      {"nd", "Latn"},
      {"",""}, {"",""}, {"",""},
#line 150 "OXML_LangToScriptConverter.gperf"
      {"nn", "Latn"},
#line 195 "OXML_LangToScriptConverter.gperf"
      {"tk", "Latn"},
      {"",""}, {"",""}, {"",""},
#line 136 "OXML_LangToScriptConverter.gperf"
      {"mk", "Cyrl"},
#line 178 "OXML_LangToScriptConverter.gperf"
      {"sk", "Latn"},
      {"",""}, {"",""},
#line 149 "OXML_LangToScriptConverter.gperf"
      {"nl", "Latn"},
#line 201 "OXML_LangToScriptConverter.gperf"
      {"tt", "Cyrl"},
#line 99 "OXML_LangToScriptConverter.gperf"
      {"ie", "Latn"},
      {"",""}, {"",""},
#line 142 "OXML_LangToScriptConverter.gperf"
      {"mt", "Latn"},
#line 186 "OXML_LangToScriptConverter.gperf"
      {"st", "Latn"},
#line 199 "OXML_LangToScriptConverter.gperf"
      {"tr", "Latn"},
      {"",""},
#line 157 "OXML_LangToScriptConverter.gperf"
      {"om", "Latn"},
      {"",""},
#line 140 "OXML_LangToScriptConverter.gperf"
      {"mr", "Deva"},
#line 184 "OXML_LangToScriptConverter.gperf"
      {"sr", "Cyrl"},
      {"",""}, {"",""},
#line 125 "OXML_LangToScriptConverter.gperf"
      {"lb", "Latn"},
#line 116 "OXML_LangToScriptConverter.gperf"
      {"kn", "Knda"},
#line 47 "OXML_LangToScriptConverter.gperf"
      {"be", "Cyrl"},
      {"",""}, {"",""},
#line 93 "OXML_LangToScriptConverter.gperf"
      {"ht", "Latn"},
#line 40 "OXML_LangToScriptConverter.gperf"
      {"an", "Latn"},
#line 151 "OXML_LangToScriptConverter.gperf"
      {"no", "Latn"},
      {"",""}, {"",""},
#line 114 "OXML_LangToScriptConverter.gperf"
      {"kl", "Latn"},
#line 92 "OXML_LangToScriptConverter.gperf"
      {"hr", "Latn"},
#line 111 "OXML_LangToScriptConverter.gperf"
      {"ki", "Latn"},
      {"",""}, {"",""},
#line 87 "OXML_LangToScriptConverter.gperf"
      {"gv", "Latn"},
#line 154 "OXML_LangToScriptConverter.gperf"
      {"ny", "Latn"},
#line 200 "OXML_LangToScriptConverter.gperf"
      {"ts", "Latn"},
      {"",""}, {"",""},
#line 132 "OXML_LangToScriptConverter.gperf"
      {"lv", "Latn"},
#line 141 "OXML_LangToScriptConverter.gperf"
      {"ms", "Latn"},
#line 185 "OXML_LangToScriptConverter.gperf"
      {"ss", "Latn"},
      {"",""}, {"",""},
#line 115 "OXML_LangToScriptConverter.gperf"
      {"km", "Khmr"},
      {"",""},
#line 148 "OXML_LangToScriptConverter.gperf"
      {"ng", "Latn"},
      {"",""}, {"",""},
#line 39 "OXML_LangToScriptConverter.gperf"
      {"am", "Ethi"},
#line 59 "OXML_LangToScriptConverter.gperf"
      {"cr", "Cans"},
#line 117 "OXML_LangToScriptConverter.gperf"
      {"ko", "Hang"},
      {"",""},
#line 162 "OXML_LangToScriptConverter.gperf"
      {"pl", "Latn"},
#line 122 "OXML_LangToScriptConverter.gperf"
      {"kw", "Latn"},
#line 161 "OXML_LangToScriptConverter.gperf"
      {"pi", "Deva"},
#line 144 "OXML_LangToScriptConverter.gperf"
      {"na", "Latn"},
      {"",""}, {"",""},
#line 37 "OXML_LangToScriptConverter.gperf"
      {"af", "Latn"},
#line 123 "OXML_LangToScriptConverter.gperf"
      {"ky", "Cyrl"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 44 "OXML_LangToScriptConverter.gperf"
      {"ay", "Latn"},
#line 187 "OXML_LangToScriptConverter.gperf"
      {"su", "Latn"},
      {"",""}, {"",""}, {"",""},
#line 70 "OXML_LangToScriptConverter.gperf"
      {"en", "Latn"},
#line 110 "OXML_LangToScriptConverter.gperf"
      {"kg", "Latn"},
      {"",""}, {"",""}, {"",""},
#line 60 "OXML_LangToScriptConverter.gperf"
      {"cs", "Latn"},
#line 102 "OXML_LangToScriptConverter.gperf"
      {"ik", "Latn"},
      {"",""}, {"",""},
#line 69 "OXML_LangToScriptConverter.gperf"
      {"el", "Grek"},
#line 94 "OXML_LangToScriptConverter.gperf"
      {"hu", "Latn"},
#line 109 "OXML_LangToScriptConverter.gperf"
      {"ka", "Geor"},
      {"",""}, {"",""},
#line 96 "OXML_LangToScriptConverter.gperf"
      {"hz", "Latn"},
#line 105 "OXML_LangToScriptConverter.gperf"
      {"it", "Latn"},
#line 34 "OXML_LangToScriptConverter.gperf"
      {"aa", "Ethi"},
      {"",""}, {"",""}, {"",""},
#line 145 "OXML_LangToScriptConverter.gperf"
      {"nb", "Latn"},
#line 147 "OXML_LangToScriptConverter.gperf"
      {"ne", "Deva"},
      {"",""}, {"",""},
#line 217 "OXML_LangToScriptConverter.gperf"
      {"zh", "Hans"},
#line 77 "OXML_LangToScriptConverter.gperf"
      {"fi", "Latn"},
#line 205 "OXML_LangToScriptConverter.gperf"
      {"uk", "Cyrl"},
      {"",""}, {"",""},
#line 130 "OXML_LangToScriptConverter.gperf"
      {"lt", "Latn"},
#line 61 "OXML_LangToScriptConverter.gperf"
      {"cu", "Cyrl"},
#line 71 "OXML_LangToScriptConverter.gperf"
      {"eo", "Latn"},
      {"",""}, {"",""}, {"",""},
#line 160 "OXML_LangToScriptConverter.gperf"
      {"pa", "Guru"},
#line 54 "OXML_LangToScriptConverter.gperf"
      {"br", "Latn"},
      {"",""}, {"",""}, {"",""},
#line 153 "OXML_LangToScriptConverter.gperf"
      {"nv", "Latn"},
#line 206 "OXML_LangToScriptConverter.gperf"
      {"ur", "Arab"},
      {"",""}, {"",""}, {"",""},
#line 79 "OXML_LangToScriptConverter.gperf"
      {"fo", "Latn"},
#line 104 "OXML_LangToScriptConverter.gperf"
      {"is", "Latn"},
      {"",""},
#line 76 "OXML_LangToScriptConverter.gperf"
      {"ff", "Latn"},
      {"",""},
#line 35 "OXML_LangToScriptConverter.gperf"
      {"ab", "Cyrl"},
#line 36 "OXML_LangToScriptConverter.gperf"
      {"ae", "Avst"},
      {"",""}, {"",""},
#line 81 "OXML_LangToScriptConverter.gperf"
      {"fy", "Latn"},
      {"",""},
#line 165 "OXML_LangToScriptConverter.gperf"
      {"qu", "Latn"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 55 "OXML_LangToScriptConverter.gperf"
      {"bs", "Latn"},
      {"",""}, {"",""}, {"",""},
#line 121 "OXML_LangToScriptConverter.gperf"
      {"kv", "Cyrl"},
#line 216 "OXML_LangToScriptConverter.gperf"
      {"za", "Latn"},
      {"",""}, {"",""}, {"",""},
#line 43 "OXML_LangToScriptConverter.gperf"
      {"av", "Cyrl"},
#line 106 "OXML_LangToScriptConverter.gperf"
      {"iu", "Cans"},
      {"",""}, {"",""}, {"",""},
#line 75 "OXML_LangToScriptConverter.gperf"
      {"fa", "Arab"},
#line 64 "OXML_LangToScriptConverter.gperf"
      {"da", "Latn"},
      {"",""}, {"",""}, {"",""},
#line 86 "OXML_LangToScriptConverter.gperf"
      {"gu", "Gujr"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 131 "OXML_LangToScriptConverter.gperf"
      {"lu", "Latn"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 169 "OXML_LangToScriptConverter.gperf"
      {"ru", "Cyrl"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 207 "OXML_LangToScriptConverter.gperf"
      {"uz", "Cyrl"},
#line 68 "OXML_LangToScriptConverter.gperf"
      {"ee", "Latn"},
      {"",""}, {"",""}, {"",""},
#line 158 "OXML_LangToScriptConverter.gperf"
      {"or", "Orya"},
      {"",""}, {"",""}, {"",""}, {"",""}, {"",""},
#line 152 "OXML_LangToScriptConverter.gperf"
      {"nr", "Latn"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 65 "OXML_LangToScriptConverter.gperf"
      {"de", "Latn"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 113 "OXML_LangToScriptConverter.gperf"
      {"kk", "Cyrl"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 38 "OXML_LangToScriptConverter.gperf"
      {"ak", "Latn"},
      {"",""}, {"",""}, {"",""},
#line 159 "OXML_LangToScriptConverter.gperf"
      {"os", "Cyrl"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 66 "OXML_LangToScriptConverter.gperf"
      {"dv", "Thaa"},
#line 118 "OXML_LangToScriptConverter.gperf"
      {"kr", "Arab"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 41 "OXML_LangToScriptConverter.gperf"
      {"ar", "Arab"},
      {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},
      {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},
#line 164 "OXML_LangToScriptConverter.gperf"
      {"pt", "Latn"},
      {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},
#line 119 "OXML_LangToScriptConverter.gperf"
      {"ks", "Arab"},
      {"",""}, {"",""}, {"",""},
#line 156 "OXML_LangToScriptConverter.gperf"
      {"oj", "Cans"},
#line 42 "OXML_LangToScriptConverter.gperf"
      {"as", "Beng"},
      {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},
      {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},
      {"",""},
#line 73 "OXML_LangToScriptConverter.gperf"
      {"et", "Latn"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 163 "OXML_LangToScriptConverter.gperf"
      {"ps", "Arab"},
#line 120 "OXML_LangToScriptConverter.gperf"
      {"ku", "Arab"},
      {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},
      {"",""}, {"",""},
#line 45 "OXML_LangToScriptConverter.gperf"
      {"az", "Latn"},
#line 112 "OXML_LangToScriptConverter.gperf"
      {"kj", "Latn"},
      {"",""}, {"",""}, {"",""},
#line 80 "OXML_LangToScriptConverter.gperf"
      {"fr", "Latn"},
      {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 72 "OXML_LangToScriptConverter.gperf"
      {"es", "Latn"},
      {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},
      {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},
      {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},
      {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},
#line 74 "OXML_LangToScriptConverter.gperf"
      {"eu", "Latn"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 218 "OXML_LangToScriptConverter.gperf"
      {"zu", "Latn"},
      {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},
      {"",""}, {"",""}, {"",""}, {"",""}, {"",""}, {"",""},
      {"",""},
#line 67 "OXML_LangToScriptConverter.gperf"
      {"dz", "Tibt"},
      {"",""}, {"",""}, {"",""}, {"",""},
#line 78 "OXML_LangToScriptConverter.gperf"
      {"fj", "Latn"}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].lang;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}
