/**************************************************************
    JHUFF.C -- A rle/huffman Data Compression Program
**************************************************************/

typedef struct
{
  unsigned short bit0,bit1;	// 0-255 is a character, > is a pointer to a node
} huffnode;

long counts[256];

unsigned huffbits[256];
unsigned long huffstring[256];

huffnode nodearray[256];	// 256 nodes is worst case

/*
=============================================================================

			   COMPRESSION SUBS

=============================================================================
*/



/*
======================
=
= CountBytes
=
= Adds the bytes in the pointed to area to the counts array
= If this is the first segment, make sure counts is zerod
=
======================
*/

void CountBytes (unsigned char *start, long length)
{
  long i;

  while (length--)
    counts[*start++]++;
}

/*
=======================
=
= FindLeast
=
= Returns the byte with the lowest counts value
=
=======================
*/

int FindLeast (void)
{
  int i,least;
  long low = 0x7fffffff;

  for (i=0;i<256;i++)
    if (counts[i]<low)
    {
      low = counts[i];
      least = i;
    }

  return least;
}

/*========================================================================*/

/*
==================
=
= TraceNode
=
= A recursive function that follows all leaves of nodearray and fills in
= coding tables huffbits and huffstring.
=
==================
*/

void TraceNode (int nodenum,int numbits,unsigned long bitstring)
{
  unsigned bit0,bit1;

  bit0 = nodearray[nodenum].bit0;
  bit1 = nodearray[nodenum].bit1;

  numbits++;


  if (bit0 <256)
  {
    huffbits[bit0]=numbits;
    huffstring[bit0]=bitstring;		// just added a zero in front
  }
  else
  {
    if (numbits<24)			// if the string is this long, its 0
      TraceNode (bit0-256,numbits,bitstring);
  }

  if (bit1 <256)
  {
    huffbits[bit1]=numbits;
    huffstring[bit1]=bitstring+ (1ul<<(numbits-1));	// add a one in front
    if (huffbits[bit1]>24 && counts[bit1])
    {
      puts("Error: Huffman bit string went over 32 bits!");
      exit(1);
    }
  }
  else
  {
    if (numbits<24)			// if the string is this long, its 0
	 TraceNode (bit1-256,numbits,bitstring+(1ul<<(numbits-1)));
  }
}


/*
=======================
=
= Huffmanize
=
= Takes the counts array and builds a huffman tree at
= nodearray, then builds a codeing table.
= 
=======================
*/

void Huffmanize (void)
{
//
// codes are either bytes if <256 or nodearray numbers+256 if >=256
//
  unsigned value[256],code0,code1;
//
// probablilities are the number of times the code is hit or $ffffffff if
// it is allready part of a higher node
//
  unsigned long prob[256],low,workprob;

  int i,worknode,bitlength;
  unsigned long bitstring;

  memset(huffstring,0,sizeof(huffstring));
  memset(huffbits,0,sizeof(huffbits));

//
// all possible leaves start out as bytes
//
  for (i=0;i<256;i++)
  {
    value[i]=i;
    prob[i]=counts[i];
  }

//
// start selecting the lowest probable leaves for the ends of the tree
//

  worknode = 0;
  while (1)	// break out of when all codes have been used
  {
    //
    // find the two lowest probability codes
    //

    code0=0xffff;
    low = 0x7ffffffff;
    for (i=0;i<256;i++)
      if (prob[i]<low)
      {
	code0 = i;
	low = prob[i];
      }

    code1=0xffff;
    low = 0x7fffffff;
    for (i=0;i<256;i++)
      if (prob[i]<low && i != code0)
      {
	code1 = i;
	low = prob[i];
      }

    if (code1 == 0xffff)
    {
      if (value[code0]<256)
	puts("Weirdo huffman error: last code wasn't a node!");
      if (value[code0]-256 != 254)
	puts("Weirdo huffman error: headnode wasn't 254!");
      break;
    }

    //
    // make code0 into a pointer to work
    // remove code1 (make 0xffffffff prob)
    //
    nodearray[worknode].bit0 = value[code0];
    nodearray[worknode].bit1 = value[code1];

    value[code0] = 256 + worknode;
    prob[code0] += prob[code1];
    prob[code1] = 0xffffffff;
    worknode++;
  }

//
// done with tree, now build table recursively
//

  TraceNode (254,0,0);

}

/*========================================================================*/

/*
===============
=
= OptimizeNodes
=
= Goes through a huffman table and changes the 256-511 node numbers to the
= actular address of the node.  Must be called before HuffExpand
=
===============
*/

void OptimizeNodes (huffnode *table)
{
  huffnode *node;
  int i;

  node = table;

  for (i=0;i<255;i++)
  {
    if (node->bit0 >= 256)
      node->bit0 = (unsigned)(table+(node->bit0-256));
    if (node->bit1 >= 256)
      node->bit1 = (unsigned)(table+(node->bit1-256));
    node++;
  }
}

/*========================================================================*/

/*
======================
=
= HuffCompress
=
= The file must be counted with CountBytes and then Huffmanized first
=
======================
*/

long HuffCompress (unsigned char *source, long length,
  unsigned char *dest)
{
  long outlength;
  unsigned long string;
  unsigned biton,bits;
  unsigned char byte;

  outlength = biton = 0;

  *(long *)dest=0;		// so bits can be or'd on

  while (length--)
  {
	byte = *source++;
	bits = huffbits[byte];
	string = huffstring[byte] << biton;
	*(long *)(dest+1)=0;	// so bits can be or'd on
	*(long *)dest |= string;
	biton += bits;		// advance this many bits
	dest+= biton/8;
	biton&=7;			// stay under 8 shifts
	outlength+=bits;
  }

  return (outlength+7)/8;
}

/*========================================================================*/

/*
======================
=
= HuffExpand
=
======================
*/

void HuffExpand (unsigned char *source, unsigned char *dest,
  long length,huffnode *hufftable)
{
  unsigned bit,byte,node,code;
  unsigned sourceseg,sourceoff,destseg,destoff,endseg,endoff;
  huffnode *nodeon,*headptr;

  nodeon = headptr = hufftable+254;	// head node is allways node 254

  bit = 1;
  byte = *source++;

  while (length)
  {
    if (byte&bit)
      code = nodeon->bit1;
    else
      code = nodeon->bit0;

    bit<<=1;
    if (bit==256)
    {
      bit=1;
      byte = *source++;
    }

    if (code<256)
    {
      *dest++=code;
      nodeon=headptr;
      length--;
    }
    else
      nodeon = (huffnode *)code;
  }


}
