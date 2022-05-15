namespace screen
{
    char posX = 0;
    char posY = 0;
    char attrib = 0x07;
    #define RAMSCREEN 0xB8000
    void clear()
    {
        for (posY = 0; posY < 25; posY++)
        {
            for (posX = 0; posX < 80; posX++)
            {
                unsigned char *ramchar = (unsigned char *)RAMSCREEN + 2 * posX + 160 * posY;
                *ramchar = 0;
            }
        }
        posX = 0;
        posY = 0;
    }

    void printchar(unsigned char input)
    {
        unsigned char *ramchar = (unsigned char *)RAMSCREEN + 2 * posX + 160 * posY;
        *(ramchar + 1) = attrib;
        if (input == 10) // CR-NL
        {
            posX = 0;
            posY++;
        }
        else if (input == 9) // TAB
        {
            posX = posX + 8 - (posX % 8);
        }
        else if (input == 13) // CR
        {
            posX = 0;
        }
        else
        {
            *ramchar = input;
            posX++;
            if (posX > 79)
            {
                posY++;
                posX = 0;
            }
        }

        if (posY > 24)
        {
            // We gotta scroll at this point
            clear(); // Let's just do that for now
            posY = 0;
            posX = 0;
            //scrollup(1);
        }
    }

    void print(char *str)
    {
	    while(*str)
	    {
		    printchar(*str);
		    str++;
	    }
    }
}