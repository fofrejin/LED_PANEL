// ****** UTF8-Decoder: convert UTF8-string to extended ASCII *******
static byte c1;  // Last character buffer

// Convert a single Character from UTF8 to Extended ASCII
// Return "0" if a byte has to be ignored
byte utf8ascii(byte ascii) {
    if ( ascii<128 )   // Standard ASCII-set 0..0x7F handling  
    {   c1=0; 
        return( ascii ); 
    }

    // get previous input
    byte last = c1;   // get last char
    c1=ascii;         // remember actual character

    switch (last)     // conversion depending on first UTF8-character(byte)
    {   //case 0xC2: return  (ascii);  break;
        case 0xC2: if(ascii==0xB0) return (0x8A); else return (ascii);  break; //ASCII and Degree sign ° 138
        case 0xD0: return  (ascii);  break; //UTF-8 Kirill А-п
        case 0xD1: return  (ascii+64);  break; //UTF-8 Kirill р-я
        case 0xD2: return  (ascii+66);  break; //UTF-8 Kirill Ґґ
        case 0xC3: return  (ascii | 0xC0);  break;
    }
    return  (0);                                     // otherwise: return zero, if character has to be ignored
}

// convert String object from UTF8 String to Extended ASCII
String utf8ascii(String s)
{       
        String r="";
        char c;
        for (int i=0; i<s.length(); i++)
        {
                c = utf8ascii(s.charAt(i));
                if (c!=0) r+=c;
        }
        return r;
}

// In Place conversion UTF8-string to Extended ASCII (ASCII is shorter!)
void utf8ascii(char* s)
{       
        int k=0;
        char c;
        for (int i=0; i<strlen(s); i++)
        {
                c = utf8ascii(s[i]);
                if (c!=0) 
                        s[k++]=c;
        }
        s[k]=0;
}
