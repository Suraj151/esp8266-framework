/*
 * Copyright (c) 1995, 1996, 1997 Kungliga Tekniska Hgskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the Kungliga Tekniska
 *      Hgskolan and its contributors.
 *
 * 4. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Base64.h"

// Character set of base64 encoding scheme
static char char_set[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Takes string to be encoded as input
// and its length and returns encoded string
char* base64Encode(char input_str[], int len_str)
{
  // Resultant string
  char *res_str = (char *) malloc(500 * sizeof(char));

  int index, no_of_bits = 0, padding = 0, val = 0, count = 0, temp;
  int i, j, k = 0;

  // Loop takes 3 characters at a time from
  // input_str and stores it in val
  for (i = 0; i < len_str; i += 3)
    {
      val = 0, count = 0, no_of_bits = 0;

      for (j = i; j < len_str && j <= i + 2; j++)
      {
        // binary data of input_str is stored in val
        val = val << 8;

        // (A + 0 = A) stores character in val
        val = val | input_str[j];

        // calculates how many time loop
        // ran if "MEN" -> 3 otherwise "ON" -> 2
        count++;

      }

      no_of_bits = count * 8;

      // calculates how many "=" to append after res_str.
      padding = no_of_bits % 3;

      // extracts all bits from val (6 at a time)
      // and find the value of each block
      while (no_of_bits != 0)
      {
        // retrieve the value of each block
        if (no_of_bits >= 6)
        {
          temp = no_of_bits - 6;

          // binary of 63 is (111111) f
          index = (val >> temp) & 63;
          no_of_bits -= 6;
        }
        else
        {
          temp = 6 - no_of_bits;

          // append zeros to right if bits are less than 6
          index = (val << temp) & 63;
          no_of_bits = 0;
        }
        res_str[k++] = char_set[index];
      }
  }

  // padding is done here
  for (i = 1; i <= padding; i++)
  {
    res_str[k++] = '=';
  }

  res_str[k] = '\0';

  return res_str;

}
