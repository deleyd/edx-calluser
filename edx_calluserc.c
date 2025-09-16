/* EDX_CALLUSERC.C - externcal CALL_USER routines for EDX.
   This module is used in conjunction with the EDX editor.
   This module is intended to be compatible with both VAX/VMS VAX C
   and Alpha AXP/OpenVMS DEC C */
#if 0
ASSUME - THIS MODULE ASSUMES THE FOLLOWING:
         char *pointer;
         int  integer32;
         char *pointer;   sizeof(pointer) = longword (4 bytes, 32 bits)
         int  i;          sizeof(i)       = longword (4 bytes, 32 bits)
         long int j;      sizeof(j)       = longword (4 bytes, 32 bits)
         short int k;     sizeof(k)       = word     (2 bytes, 16 bits)


   This module is called using the TPU command:

      OUTSTR := CALL_USER( CODE, INSTR);

psudo-c call:
      status = tpu$calluser( code, instr, outstr );

 CODE - Integer.  Input (read only), passed by reference.

        INPUT REQUEST CODE:
        The input request code is split into words.  The high word indicates
        the general category of the request, the low word indicates the
        specific request within the category. This simplifies parsing.

 INSTR - Input string, read only, passed by descriptor.
         8(AP)  = address of string descriptor.
         The string is passed by a fixed length descriptor of the form:

         -----------------------------------------
         |  class  |  dtype  |  string length    |
         -----------------------------------------
         |                address                |
         -----------------------------------------

          length  = nn  (length of string)
          dtype   = 14  (character string DSC$K_DTYPE_T)
          class   =  0  (DSC$K_CLASS_Z unspecified.  This doesn't work for some things)
          address = xx  (address of first byte of string)

        Because DSC$K_CLASS_Z doesn't work for some library calls, we create
        our own string descriptor.  The length and address fields are copied
        over, dtype is set to DSC$K_DTYPE_T and class is set to DSC$K_CLASS_S.


 OUTSTR - Output string, write only, by descriptor.
          The string is passed by a dynamic string descriptor where:

           length  =  0  (length of string)
           dtype   = 14  (character string DSC$K_DTYPE_T)
           class   =  2  (dynamic string descriptor DSC$K_CLASS_D)
           address =  0  (invalid address)

            Because the result string is dynamically allocated we use
            the system service run-time library routine STR$COPY_DX
            to return the string.

            The first 9 characters of the output string are reserved
            for RETCODE, the return status code number.  The calling program
            strips the first 9 characters off the string and converts
            it to a return status code number using the INT() function.

 RETURN STATUS
      The return value of this function tpu$calluser.  This module
      always returns a SS$_NORMAL status when it returns.  The only
      way an error status is returned is if an unexpected nasty
      error happens.  If an error status is returned TPU will take
      the ON_ERROR - ENDON_ERROR action if one exists in the calling
      procedure.

; INPUT ITEM CODE CATEGORIES:
; ^x0001xxxx - SYSTEM                   (65536)
; ^x00010001 - LOCK FILE                (65537)
;              INSTR = filename
;
; ^x00010002 - UNLOCK FILE              (65538)
;              INSTR = filename
;
; ^x00010003 - SHOW LOGICAL             (65539)
;              INSTR = logical name to translate
;              OUTSTR = logical name translation
;
; ^x00010004 - SHOW SYMBOL              (65540)
;              INSTR = DCL symbol to translate
;              OUTSTR = symbol translation
;
; ^x00010005 - PRINT ERROR MESSAGE      (65541) (NO LONGER USED)
;              INSTR = string containing error number (in decimal)
;
; ^x00010006 - CHECK IF FILE IS LOCKED  (65542)
;              INSTR = string containing filename to check
;              Return INCODE = (1 = file locked, 0 file not locked)
;
; ^x00010007 - SET DEFAULT DIRECTORY    (65543)
;              INSTR = string containing new directory to go to.
;
; ^x00010008 - DEFINE LOGICAL NAME      (65544)
;              INSTR = string containing logical name followed by translation.
;                      The string is of the form "log-nam  value", where "log-nam"
;                      is the logical name to be defined and "value" is the
;                      value to be assigned to the logical name.  One or more
;                      spaces must separate the two.
;
; ^x00010009 - SHOW IDENT NUMBER        (65545)
;              OUTSTR = Ident version number.
;
; ^x0001000A - DELETE FILE              (65546)
;              INSTR = Filename to delete.
;
; ^x0001000B - SET SYMBOL               (65547)
;              INSTR = symbol to set
;
; ^x0001000C - CALC                     (65548)   (currently not used)
;              INSTR = math expression
;
;
; ^x0002000n - SENDING MESSAGE FLAGS    (131072)
;          n - Value of message flags setting
;              This code is used when we recursively call ourselves to
;              give ourselves the current value (0-15) of message flags.
;              Used when a message is signaled.
;
;
; ^x0003000n - DIRECTORY                (196608)
;          n - Code used for reentry.
;
;
; ^x00040001 - TRANSLATE FROM EBCDIC TO ASCII   (262145)
;              INSTR = EBCDIC string
;              OUTSTR = ASCII string
;
; ^x00040002 - TRANSLATE FROM ASCII TO EBCDIC   (262146)
;              INSTR = ASCII string
;              OUTSTR = EBCDIC string
;
; ^x00040003 - INITIALIZE RANDOM NUMBER GENERATOR WITH PASSWORD (262147) (NOT USED)
;              INSTR = Password
;              OUTSTR = Status
;
; ^x00040004 - ENCRYPT STRING                   (262148)  (NOT USED)
;              INSTR = String to encrypt
;              OUTSTR = Encrypted string
;
; ^x00040005 - DECRYPT STRING                   (262149)  (NOT USED)
;              INSTR = String to decrypt
;              OUTSTR = Decrypted string
;
; ^x0005000n - SORT                     (327680)
;          n = 1.  Preparse command line
;              2.  Pass files and do sort (for file sort)
;              3.  Postparse command line
;              4.  Pass a record to sort.  (Repeat until all records passed)
;              5.  Do record sort
;              6.  Receive a record in sorted order.  (Repeat until all records received)
;              7.  Cleanup record sort
;
; ^x0006000n - SPELL            (393216)
;          n = 1.  Dictionary browse previous page
;          n = 2.  Dictionary browse using word
;          n = 3.  Dictionary browse next page
;          n = 4.  Spell textline
;          n = 5.  Spell guess
;          n = 6.  Accept word (add to accepted word list)
;          n = 7.  Add word to personal dictionary.
;          n = 8.  Dump commonwords list
;          n = 9.  Save misspelled word and its correction
;
; ^x0007000n - LIBRARY          (458752)
;          n = 1.  Initialize, open for read, lookup_key
;          n = 2.  Return next line of text from module
;          n = 3.  Close text library
;          n = 4.  Initialize, open for write, lookup_key
;          n = 5.  Write next line of text to module
;          n = 6.  Write end-of-module record
;
;#if DEBUG
; ^x1001000n - HANDLER_TEST     (268500992)
;          n = 1.  Generate access violation
;          n = 2.  Generate divide by zero exception
;          n = 3.  Return TPU$GL_VERSION
;          n = 4.  Return TPU$GL_UPDATE
;          n = 5.  signal EDX__UNKNCODE
;#endif
;
;  System routines
;  TPU$CALLUSER                 !Main entry point.  Entered via TPU CALL_USER instruction.
;  SHOW_ID                      !Show ident number
;  FMTOUTSTR                    !Format output string
;  EDX_SIGNAL                   !Signal message
;  HANDLER                      !Error handler
;
;  Display directory listing
;  EDX_DIRECTORY                !Display directory listing
;  GETDEFDIRFLGS                ! support for directory command
;
;Sort Routines:
;  EDX_SORT                     !Main entry.
;  SORT_PREPARSE                !Preparse SORT command
;  SORT_PASSFILES               !Pass filenames for file sort
;  SORT_POSTPARSE               !Finish parsing SORT command
;  SORT_DO_FILE                 !Do file sort
;  SORT_RELEASE_REC             !Give record to sort when using record sort
;  SORT_RETURN_REC              !Get record from sort when using record sort
;
;  Spelling checker and dictionary
;  EDX_SPELL                    !Spelling dictionary main entry
;  SPELL_INIT                   !Initialize spelling checker
;  SPELL_TEXTLINE               !Spell check a line of text
;  DIC_LOOKUP_WORD              !Look up a word in the dictionary
;  DIC_BROWSE                   !Browse through the dictionary
;  DIC_BROWSE_PREV_PAGE         ! support for browse
;  DIC_BROWSE_WORD              ! support for browse
;  DIC_BROWSE_FILL              ! support for browse
;  SPELL_GUESS                  ! Guess the spelling of a word.  From Vassar.
;  SPELL_ACCEPT_WORD            ! Insert word into accepted word tree list
;  SAVE_CORRECTION              ! Save misspelled word and its correction
;  PRINT_ACCEPT_NODE            ! debug routine for accepted tree list
;  PRINT_SAVCOR_NODE            ! debug routine for accepted tree list
;  TRAVERSE_ACCEPT_TREE         ! debug routine for accepted tree list
;  TRAVERSE_SAVCOR_TREE         ! debug routine for saved corrections tree list
;  ALLOCATE_NODE                ! support routine for accepted tree list
;  COMPARE_NODE                 ! support routine for accepted tree list
;  SPELL_PERSDIC_ADD            ! add word to personal dictionary
;  DUMP_COMMONWORDS             ! dump the commonword list
;
;  Lock and unlock files
;  LOCK_FILE                    !Lock a file preventing others from editing it
;  UNLOCK_FILE                  !Unlock file
;  EDX_CKFILK                   !Check if file is locked
;  SRCH_LNKFABLST               !Search our list of locked files
;  EDX_PARSE                    !Parse a filename
;
;  Miscellaneous
;  EDX_SETDEF                   !Change users default directory
;  SET_LOGICAL                  !Create a logical name
;  SET_SYMBOL                   !Create a DCL symbol
;  SHOW_LOGICAL                 !Show translation of a logical name
;  SHOW_SYMBOL                  !Show translation of a DCL symbol
;  DELETE_FILE                  !Delete a file
;  TRA_EBC_ASC                  !Translate EBCDIC to ASCII
;  TRA_ASC_EBC                  !Translate ASCII to EBCDIC
;
;  Text libraries
;  LIBRARIAN
;  LBR_INIT
;  LBR_CLOSE
;  LBR_READNEXT
;  LBR_WRITENEXT
;  LBR_REPLACE
;
;  David Deley  May, 1988  Original
;  David Deley  Nov. 1988  New version compatible with VMS 5.0
;  David Deley  Nov. 1989  v5.7 with string sort
;  David Deley  Mar. 1990  V6.0 with spelling checker dictionary
;  David Deley  Sep. 1990  V7.2 more spelling checker and fixes for VMS 5.3 bug
;  David Deley  Dec. 1991  V8.1 DES encryption removed.
;  David Deley  Feb. 1992  V8.2 Spell check 'Accept' could fail with access violation in ALLOCATE_NODE (caught by HANDLER)
;  David Deley  Nov. 1992  V8.4 Sort default key length 132 -&gt; 65535
;                               Properly signal SOR$_BAD_LRL condition
; David Deley   Mar. 1993  V8.6 No changes.
; David Deley   Jul. 1993  V9.0 (474) Converted to C.
; David Deley   Nov. 1993  V9.0 (475) Fix add_persdic() invalid RAB

C programming notes:
        char outline
        subr( outline     );    PASSED BY REFERENCE
        subr( outline[0]  );    PASSED BY VALUE
        subr( &outline[0] );    PASSED BY REFERENCE
        ("string")              PASSED BY REFERENCE
        != takes precedence over & so use parentheses
        ~ - Bitwise .NOT. (Ones Complement)
        ! - Logical .NOT. (TRUE/FALSE)
        && - Logical AND
        &  - Bitwise .AND.

   ptr = (char *)(node+12);
         Take the value of 'node', add 12, claim the result is a pointer
         to a stream of characters, set 'ptr', a variable of type "pointer
         to a stream of characters" to that value.

   end = ptr + *(unsigned short *)(node+10);
         Take the value of 'node', add 10, claim the result is a pointer
         to an unsigned short word.  Take the value of that unsigned short
         word, add it to 'ptr', place in 'end'.


        char *fab$l_fna;                                        * defined as "pointer to char" *
        (*NO*)  lexfab.fab$l_fna = &"WORDLIST:";                * DEC C complains "pointer to array of char " incompatible *
        (YES)   lexfab.fab$l_fna = (char *) "WORDLIST:";        * DEC C accepts *

        char wdbuf[80];
        char *rab$l_ubf;                                        * defined as "pointer to char" *
        (*NO*)  cwrab.rab$l_ubf = &wdbuf;                       * DEC C complains *
        (YES)   cwrab.rab$l_ubf = (char *) wdbuf;               * DEC C accepts *

        char (*ctable)[];       * pointer to an array of char *

Fields to watch for above error:
FAB:    char *fab$l_xab;???             * xab address                      *
        char *fab$l_fna;                * file name string address         *
        char *fab$l_dna;                * default file name string addr    *
RAB:    char *rab$l_ubf;                * user buffer address              *
        char *rab$l_rbf;                * record buffer address            *
        char *rab$l_rhb;                * record header buffer addr        *
        char *rab$l_kbf;                * key buffer address               *
        char *rab$l_pbf;                * prompt buffer addr               *
        char *rab$l_xab;???             * XAB address                      *
NAM:    char *nam$l_rsa;                * resultant string area address    *
        char *nam$l_esa;                * expanded string area address     *
        char *nam$l_node;               * Nodespec address                 *
        char *nam$l_dev;                * Device address                   *
        char *nam$l_dir;                * Directory address                *
        char *nam$l_name;               * Filename address                 *
        char *nam$l_type;               * Filetype address                 *
        char *nam$l_ver;                * Version number address           *
XAB:    char *xab$l_nxt;???             * Next XAB in chain *

ALL global variables declared outside of a routine must be 'static'
otherwise they end up in a psect of type SHAREABLE and WRITABLE
and we get the error:
  SYSTEM-F-NOTINSTALL, writable shareable images must be installed

const constants are not constant unless they are also static.
When DICID was declared const without static we got DICHEADERR
/*---------------------------------------------------------------------------*/
/* A REVIEW OF C POINTERS */
#include rms

struct locked_file
{
  struct locked_file *next;
  struct FAB fab;
  struct NAM nam;
  char expanded_file_name[NAM$C_MAXRSS];
  char resultant_file_name[NAM$C_MAXRSS];
};

static struct locked_file *locked_files_head = 0;       /* head of the singulary linked list */
static struct locked_file blk;

int srch( struct locked_file **prelnkptradr )   /* pointer to a pointer to a */
{                                               /* structure of type locked_file */
   int a,b,c,d;
   a = prelnkptradr;    /* address of prelnkptr.  (value of prelnkptradr) */
   b = *prelnkptradr;   /* value of prelnkptr.  (address of blk) */
   c = (*prelnkptradr)-&gt;next;   /* c = blk.next */
   d = &prelnkptradr;   /* address of {address of prelnkptr} in call
                           frame argument list.  (SHOW SYMBOL/ADDRESS
                           prelnkptradr). */
}

main()
{
   int i,j,k;
   struct locked_file *prelnkptr;

   prelnkptr = &blk;
   i = prelnkptr;               /* value of prelnkptr.  (address of blk) */
   prelnkptr-&gt;next = 0x1234;    /* blk.next = 0x1234 */
   j = prelnkptr-&gt;next;         /* j = blk.next */
   k = &prelnkptr;              /* address of variable prelnkptr */
   srch( &prelnkptr );          /* pass by reference */
}

Update notes:
David Deley   06-OCT-1993 (469) Fix sepll_guscon
David Deley   22-NOV-1993 (477) Fix sort postparse change value_len to short

#endif
/*===========================================================================*/

#include &lt;chfdef.h&gt;             /* Include CHF$ definitions (chfdef2.h for ALPHA?) */
#include &lt;climsgdef.h&gt;
#include &lt;ctype.h&gt;              /* _toupper macro */
#include &lt;descrip.h&gt;            /* Define DSC$ and descriptor structures */
#include &lt;jpidef.h&gt;             /* Include JPI$ definitions */
#include &lt;lbrdef.h&gt;             /* Include LBR$ definitions */
#include &lt;libdef.h&gt;             /* Include LIB$ definitions */
#include &lt;lib$routines.h&gt;       /* Prototype all LIB$ routines */
#include &lt;lnmdef.h&gt;             /* Include LNM$ definitions */
#include &lt;rms.h&gt;                /* Include all FAB$, NAM$, XAB$, etc... */
#include &lt;secdef.h&gt;             /* Include SEC$ definitions */
#include &lt;signal.h&gt;             /* Define VAXC$ESTABLISH */
#include &lt;sor$routines.h&gt;       /* Define SOR$ routines */
#include &lt;sordef.h&gt;             /* Define SOR$_ error messages */
#include &lt;ssdef.h&gt;              /* Include SS$ system condition code definitions */
#include &lt;starlet.h&gt;            /* Prototype all SYS$ routines */
#include &lt;stdio.h&gt;              /* Define TRUE, FALSE */
#include &lt;stdlib.h&gt;             /* Standard library */
#include &lt;string.h&gt;             /* prototype memcpy, memset */
#include &lt;stsdef.h&gt;             /* Include STS$ status value codes */

/* EXTERNAL GLOBAL VARIABLES */
globalref TPU$GL_VERSION;
globalref TPU$GL_UPDATE;
globalref edx_commands;

/* SYMBOLS WITH NO *.H FILE  (These symbols are resolved by the linker) */
globalvalue SOR$M_NOSIGNAL;             /* defined in SYS$LIBRARY:SORTSHR.EXE */
globalvalue SOR$M_STABLE;               /* defined in SYS$LIBRARY:SORTSHR.EXE */
globalvalue SOR$M_EBCDIC;               /* defined in SYS$LIBRARY:SORTSHR.EXE */
globalvalue SOR$M_MULTI;                /* defined in SYS$LIBRARY:SORTSHR.EXE */
globalvalue SOR$M_NODUPS;               /* defined in SYS$LIBRARY:SORTSHR.EXE */
globalvalue LBR$_KEYNOTFND;             /* defined in SYS$LIBRARY:LBRSHR.EXE */

/* EDX STATUS CODES  (Defined in EDX_MESSAGES.MSG) */
globalvalue EDX__ALK;
globalvalue EDX__CREPERSDIC;
globalvalue EDX__DBFDPTRLB;
globalvalue EDX__DICHEADERR;
globalvalue EDX__DICLOAD;
globalvalue EDX__DICLOADED;
globalvalue EDX__DICVERSERR;
globalvalue EDX__ERRMAPDIC;
globalvalue EDX__ERROPEN;
globalvalue EDX__ERROPENDIC;
globalvalue EDX__GUSINTERR2;
globalvalue EDX__LOCKED;
globalvalue EDX__MEMERR;
globalvalue EDX__MODNOTFND;
globalvalue EDX__NOLOCK;
globalvalue EDX__NOMODNAM;
globalvalue EDX__NOTLOCKED;
globalvalue EDX__PERSDICERR;
globalvalue EDX__SPLINITERR;
globalvalue EDX__UNEXPERR;
globalvalue EDX__UNKNCODE;
globalvalue EDX__UNLOCKED;
globalvalue EDX__WORDADD;

/* INCODES: SYSTEM STUFF */
#define INCODE_SYSTEM                   0x00010000    /*  (65536) */
#define INCODE_LOCK_FILE                0x00010001    /*  (65537) */
#define INCODE_UNLOCK_FILE              0x00010002    /*  (65538) */
#define INCODE_SHOW_LOGICAL             0x00010003    /*  (65539) */
#define INCODE_SHOW_SYMBOL              0x00010004    /*  (65540) */
/*#define INCODE_SIGNAL_ERROR_MESSAGE   0x00010005        (65541) no longer used */
#define INCODE_CHECK_IF_FILE_IS_LOCKED  0x00010006    /*  (65542) */
#define INCODE_SET_DEFAULT_DIRECTORY    0x00010007    /*  (65543) */
#define INCODE_DEFINE_LOGICAL_NAME      0x00010008    /*  (65544) */
#define INCODE_SHOW_IDENT_NUMBER        0x00010009    /*  (65545) */
#define INCODE_DELETE_FILE              0x0001000A    /*  (65546) */
#define INCODE_SET_SYMBOL               0x0001000B    /*  (65547) */
#define INCODE_CALC                     0x0001000C    /*  (65548) not used */

/* INCODES: SET MESSAGE_FLAGS */
#define INCODE_SENDING_MESSAGE_FLAGS    0x00020000    /* (131072) */

/* INCODES: DIRECTORY */
#define INCODE_DIRECTORY                0x00030000    /* (196608) */

/* INCODES: TRANSLATE */
#define INCODE_TRANSLATE                0x00040000    /* (262144) */
#define INCODE_TRANS_EBCDIC_TO_ASCII    0x00040001    /* (262145) */
#define INCODE_TRANS_ASCII_TO_EBCDIC    0x00040002    /* (262146) */
#define INCODE INIT_RANDOM              0x00040003    /* (262147) not used */
#define INCODE_ENCRYPT STRING           0x00040004    /* (262148) not used */
#define INCODE_DECRYPT_STRING           0x00040005    /* (262149) not used */

/* INCODES: SORT */
#define INCODE_SORT                     0x00050000    /* (327680) */
#define SORT_FILE_INIT                  0x00050001    /* (327681) */
#define SORT_FILE_DO                    0x00050002    /* (327682) */
#define SORT_RECORDS_INIT               0x00050003    /* (327683) */
#define SORT_RECORDS_PASS               0x00050004    /* (327684) */
#define SORT_RECORDS_DO                 0x00050005    /* (327685) */
#define SORT_RECORDS_RECV               0x00050006    /* (327686) */
#define SORT_FINISH                     0x00050007    /* (327687) */

/* INCODES: SPELL */
#define INCODE_SPELL                    0x00060000    /* (393216) */
#define INCODE_SPELL_DIC_BROWSE_PA      0x00060001    /* (393217) */
#define INCODE_SPELL_DIC_BROWSE_RW      0x00060002    /* (393218) */
#define INCODE_SPELL_DIC_BROWSE_PZ      0x00060003    /* (393219) */
#define INCODE_SPELL_TEXTLINE           0x00060004    /* (393220) */
#define INCODE_SPELL_GUESS              0x00060005    /* (393221) */
#define INCODE_SPELL_ACCEPT_WORD        0x00060006    /* (393222) */
#define INCODE_SPELL_ADD_PERSDIC        0x00060007    /* (393223) */
#define INCODE_SPELL_DUMP_COMMONWORDS   0x00060008    /* (393224) */
#define INCODE_SPELL_SAVE_CORRECTION    0x00060009    /* (393225) */

/* INCODES: LIBRARIAN */
#define INCODE_LIBRARIAN                0x00070000    /* (458752) */
#define INCODE_LIBRARIAN_OPEN_READ      0x00070001    /* (458753) */
#define INCODE_LIBRARIAN_OPEN_WRITE     0x00070002    /* (458754) */
#define INCODE_LIBRARIAN_CLOSE          0x00070003    /* (458755) */
#define INCODE_LIBRARIAN_READNEXT       0x00070004    /* (458756) */
#define INCODE_LIBRARIAN_WRITENEXT      0x00070005    /* (458757) */
#define INCODE_LIBRARIAN_REPLACE        0x00070006    /* (458758) */

#if DEBUG
/* INCODES: HANDLER_TEST */
#define INCODE_HANDLER_TEST             0x10010000    /* (268500992) */
#endif

/* SPECIAL SIZES */
#define int16 short int
#define int32 long int

/* STRUCTURES */
typedef struct
        {
           unsigned short buflen;
           unsigned short itmcod;
           unsigned int bufadr;
           unsigned int retadr;
        } item_list_3;

static struct dsc$descriptor_s s_descr;
struct dsc$descriptor_s *s_descrip(char *str_ptr)
{
   s_descr.dsc$w_length  = strlen(str_ptr);
   s_descr.dsc$b_class   = DSC$K_CLASS_S;
   s_descr.dsc$b_dtype   = DSC$K_DTYPE_T;
   s_descr.dsc$a_pointer = str_ptr;
   return &s_descr;
}

/* DESCRIPTOR FOR CONSTANTS */
struct  dsc$descriptor_sc
{
        const unsigned short    dsc$w_length;   /* length of data item in bytes */
        const unsigned char     dsc$b_dtype;    /* data type code */
        const unsigned char     dsc$b_class;    /* descriptor class code = DSC$K_CLASS_S */
        const char              *dsc$a_pointer; /* address of first byte of data storage */
};
#define $CONST_DESCRIP(name,string)     const struct dsc$descriptor_sc name = { sizeof(string)-1, DSC$K_DTYPE_T, DSC$K_CLASS_S, string }

struct locked_file
{
  struct locked_file *next;
  struct FAB fab;
  struct NAM nam;
  char expanded_file_name[NAM$C_MAXRSS];
  char resultant_file_name[NAM$C_MAXRSS];
};

static struct locked_file *locked_files_head = 0;       /* head of the singulary linked list */

/* CONSTANTS */
static const volatile char IDENTF[] = { __FILE__ } ;    /* Identify source code. */
#define IDENT  "C version (480)"                /* IDENTL */
#define SPACE  0x20                             /* Ascii space character */
#define TAB    0x09                             /* Ascii tab character */
#define BUFLEN 256              /* Usual length of string buffers (evenly divisible by 8 for quadword alignment) */
#define MAXLEN 960              /* Maximum length of line in buffer (evenly divisible by 8 for quadword alignment) */
#define SET_MESSAGE_FLAGS 2     /* Code for recursive call to set message flags */
#define FAILURE 0
#define SUCCESS 1
#define BELL 0x07
const static char bell[2] = { BELL, 0 };
const static $DESCRIPTOR(bell_desc,bell);

/* GLOBAL VARIABLES */
static struct dsc$descriptor_d *outstr_desc_ptr;        /* pointer to outstr_desc */
static long int *incode_ptr;                            /* pointer to incode */
static struct dsc$descriptor_s  instr_desc = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 };
static int msgflgs;                             /* holds current message flag setting */

/* MACROS */
#define LENGTH(c)  sizeof(c)-1          /* subtract off the trailing ASCIZ NULL character */
#define MIN(a,b)  ( ((a) &gt; (b)) ? (b) : (a) )

/* UTILITY ROUTINES */
/* string_compare implicitly null pads shorter string */
int string_compare(int str1len, char str1[], int str2len, char str2[])
{
   int i,minl;
   minl = MIN( str1len, str2len );
   for (i = 0; i &lt;= minl && str1[i] == str2[i]; ++i );
   if (i &lt; minl)
      return(str1[i] - str2[i]);
   else
      return(str1len - str2len);
}

/* pass symbolic values by reference */
int *byref( symbol )
{
    static val;
    val = symbol;
    return( &val );
}
/*---------------------------------------------------------------------------*/
/* PROTOTYPES */
/* SYSTEM */
unsigned int TPU$CALLUSER( int *incode,                 /* Main entry point.  Entered via TPU CALL_USER instruction. */
                           struct dsc$descriptor_d *instr_z,
                           struct dsc$descriptor_d *outstr  );
void tpu_calluser( int *incode,
                   struct dsc$descriptor_d *instr_z_ptr,
                   struct dsc$descriptor_d *outstr_d_ptr );
void  show_id();                                                /* Show ident number */
void  fmtoutstr( int nargs, int *sig_array );                   /* Format output string */
void  edx_signal( int nargs, int *sig_array );                  /* Signal message */
unsigned long int handler(unsigned int *sigargs,
                          unsigned int *mechargs );             /* Error handler */
#if DEBUG
void  test_handler();                                           /* Test error handler */
void  test_handler_sub();                                       /* Test error handler */
#endif

/* DIRECTORY */
void  edx_directory();                                          /* Display directory listing */
unsigned long int null_handler(unsigned int *sigargs, unsigned int *mechargs);
void  getdefdirflgs(int *dirflgs_ptr);                          /* support for directory command */

/* SPELL */
void  edx_spell();                                              /* Spelling dictionary main entry */
int   spell_init();                                             /* Initialize spelling checker */
void  spell_textline( struct dsc$descriptor_s *inbuf_desc);     /* Spell check a line of text */
int   dic_lookup_word(int wdlen, char * wdbeg);                 /* Look up a word in the dictionary */
void  binsrch_maindic( int *low, int *high, char *target_word );/* Binary search the main dictionary */
void  dic_browse();                                             /* Browse through the dictionary */
void  dic_browse_prev_page( int nchars, int nrows);             /* support for browse */
void  dic_browse_word( int inwdlen, char *inwdbeg, int window_columns, int window_rows );       /* support for browse */
void  dic_browse_fill( char *dptr, int window_columns, int window_rows );                       /* support for browse */
void  spell_guess();                                            /* Guess the spelling of a word.  From Vassar. */
int   spell_gusrev();                                           /* Guess reversals */
int   spell_gusvol();                                           /* Guess vowels */
int   spell_gusmin();                                           /* Guess minus */
int   spell_guspls();                                           /* Guess plus */
int   spell_guscon();                                           /* Guess consonants */
void  spell_accept_word();                                      /* Insert word into accepted word tree list */
void  save_correction( struct dsc$descriptor_s *str_desc );     /* Save misspelled word and its correction */
#if DEBUG
void  print_accept_node();              /* FOR DEBUGGING ONLY   debug routine for tree lists */
void  print_savcor_node();              /* FOR DEBUGGING ONLY   debug routine for tree lists */
void  traverse_accept_tree();           /* FOR DEBUGGING ONLY   debug routine for accepted tree list */
void  traverse_savcor_tree();           /* FOR DEBUGGING ONLY   debug routine for saved corrections tree list */
#endif
int   allocate_node( struct dsc$descriptor_s *symstr_desc, char **retadr,       /* support routine for accepted tree list */
                     struct dsc$descriptor_s *usrdat_desc );
int compare_node( struct dsc$descriptor_s *symstr_desc, char *treenode,         /* support routine for accepted tree list */
                  struct dsc$descriptor_s *usrdat_desc );
void  add_persdic();                                            /* add word to personal dictionary */
void  dump_commonwords();                                       /* dump the commonword list */

/* SORT */
void  edx_sort();                                               /* Main entry. */
void  sort_preparse();                                          /* Preparse SORT command */
int   sort_passfiles();                                         /* Pass filenames for file sort */
int   sort_postparse();                                         /* Finish parsing SORT command */
void  sort_do_file();                                           /* Do file sort */
void  sort_release_rec();                                       /* Give record to sort when using record sort */
void  sort_return_rec();                                        /* Get record from sort when using record sort */

/* LOCK AND UNLOCK FILES */
void  lock_file();                                              /* Lock a file preventing others from editing it */
void  unlock_file();                                            /* Unlock file */
void  edx_ckfilk();                                             /* Check if file is locked */
int srch_locked_files( struct locked_file **prelnkptradr,       /* Search our list of locked files */
                       struct locked_file **curlnkptradr );     /* (pointer to a pointer to a structure of type locked_file) */
int edx_parse( int  output_filename_buffer_length,              /* Parse a filename */
               char *output_filename_buffer,
               int  *output_filename_length,
               int  input_filename_buffer_length,
               char *input_filename_buffer,
               int  pdn );

/* MISCELLANEOUS */
void  edx_setdef();                                             /* Change users default directory */
void  set_logical();                                            /* Create a logical name */
void  set_symbol();                                             /* Create a DCL symbol */
void  show_logical();                                           /* Show translation of a logical name */
void  show_symbol();                                            /* Show translation of a DCL symbol */
void  delete_file();                                            /* Delete a file */
void  tra_ebc_asc();                                            /* Translate EBCDIC to ASCII */
void  tra_asc_ebc();                                            /* Translate ASCII to EBCDIC */

/* LIBRARIAN */
void  librarian();
void  lbr_init(int access);
void  lbr_close();
void  lbr_readnext();
void  lbr_writenext();
void  lbr_replace();

/*=============================================================================

   TITLE TPU$CALLUSER

 Functional Description:
        The sole purpose of this jacket routine is to ensure we always
        return a success status to TPU, otherwise TPU will take the
        ON_ERROR - ENDON_ERROR action if one exists or it will print
        a message "Call user routine failed with status nnnnnnnn",
*/

unsigned int TPU$CALLUSER( int *incode,
                           struct dsc$descriptor_d *instr_z_ptr,
                           struct dsc$descriptor_d *outstr_d_ptr )
{
   tpu_calluser( incode, instr_z_ptr, outstr_d_ptr );
   return(SS$_NORMAL);
}

/*---------------------------------------------------------------------------*/

void tpu_calluser( int *incode,
                   struct dsc$descriptor_d *instr_z_ptr,
                   struct dsc$descriptor_d *outstr_d_ptr )
{
   int incodeh;

   VAXC$ESTABLISH(handler);     /* Establish handler */

   /* GET INCODE.  COMPARE WITH SET_MESSAGE_FLAGS */
   incodeh = (*incode & 0xFFFF0000);
   if (incodeh == INCODE_SENDING_MESSAGE_FLAGS)
   {
      msgflgs = (*incode & 0x0000FFFF); /* low word is message flags */
      return;
   }

   /* COPY INPUT DESCRIPTOR TO GLOBAL VARIABLE also setting class */
   instr_desc.dsc$w_length  = instr_z_ptr-&gt;dsc$w_length;
   instr_desc.dsc$a_pointer = instr_z_ptr-&gt;dsc$a_pointer;

   /* SET GLOBAL VARIABLE PTR to incode */
   incode_ptr = incode;

   /* SET GLOBAL VARIABLE PTR to output descriptor */
   outstr_desc_ptr = outstr_d_ptr;

   switch (incodeh)
   {
      case INCODE_SYSTEM:
         switch(*incode)
         {
            case INCODE_LOCK_FILE:               lock_file();    return;
            case INCODE_UNLOCK_FILE:             unlock_file();  return;
            case INCODE_SHOW_LOGICAL:            show_logical(); return;
            case INCODE_SHOW_SYMBOL:             show_symbol();  return;
          /*case INCODE_SIGNAL_ERROR_MESSAGE:    edx_sigmsg();   return;  no longer used */
            case INCODE_CHECK_IF_FILE_IS_LOCKED: edx_ckfilk();   return;
            case INCODE_SET_DEFAULT_DIRECTORY:   edx_setdef();   return;
            case INCODE_DEFINE_LOGICAL_NAME:     set_logical();  return;
            case INCODE_SHOW_IDENT_NUMBER:       show_id();      return;
            case INCODE_DELETE_FILE:             delete_file();  return;
            case INCODE_SET_SYMBOL:              set_symbol();   return;
            default:  edx_signal(1, byref(EDX__UNKNCODE) ); return;
         }

      case INCODE_DIRECTORY:
         edx_directory();
         return;

      case INCODE_TRANSLATE:
         switch (*incode)
         {
            case INCODE_TRANS_EBCDIC_TO_ASCII:      tra_ebc_asc();  return;
            case INCODE_TRANS_ASCII_TO_EBCDIC:      tra_asc_ebc();  return;
            default:  edx_signal(1, byref(EDX__UNKNCODE) ); return;
         }

      case INCODE_SORT:
         edx_sort();
         return;

      case INCODE_SPELL:
         edx_spell();
         return;

      case INCODE_LIBRARIAN:
         librarian();
         return;

#if DEBUG
      case INCODE_HANDLER_TEST:
         test_handler();
         return;
#endif

      default:  edx_signal(1, byref(EDX__UNKNCODE) ); return;
   }
}
/*-----------------------------------------------------------------------------

        .SBTTL  SHOW IDENT VERSION NUMBER

 Functional Description:
        This routine returns the ident version number of this module

 Calling Sequence:
        show_id();
---------------------------------------------------------------------------*/

void show_id()
{
   int sig_array[3] = { SUCCESS, LENGTH(IDENT), IDENT };
   fmtoutstr( 3, &sig_array );
}
/*---------------------------------------------------------------------------

        .SBTTL  FMTOUTSTR

 Functional Description:
      This routine combines the return code in RETCODE with the return
      string specified in sig_array.

 Calling Sequence:
      fmtoutstr(nargs,sig_array);

 Argument inputs:
    nargs     - Number of array elements in sig_array (1 or 3 by value)
    sig_array - sig_array[0] = return code (by value)
                sig_array[1] = [length of output string] (optional, by value)
                sig_array[2] = [address of output string] (optional, by value)

 Global inputs:
    outstr_desc_ptr = address of outstr_desc

 Outputs:
    outstr_desc = output string (by descriptor).
                  output string is of the form:

                  "nnnnnnnnnSTRING..."

                  nnnnnnnnn = 9 character decimal return status for EDX
                  STRING... = string returned for EDX

----------------------------------------------------------------------------*/

void fmtoutstr( int nargs, int *sig_array )
{
   int status;
   int arg_count = 0;
   char fao_output[10];         /* 9 for length 9 field, 10th as unused trailing ASCIZ NULL */
   struct dsc$descriptor_s retstr_desc = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 };
   $DESCRIPTOR(fao_output_desc, fao_output);
   static const $DESCRIPTOR(ctloutstr1, "!9ZL");

   sys$fao( &ctloutstr1,                             /* call sys$fao with 1 argument */
            &fao_output_desc.dsc$w_length,
            &fao_output_desc,
            sig_array[0] );

   str$copy_dx( outstr_desc_ptr, &fao_output_desc );    /* copy to outstr */

   if (nargs == 3)
   {
      retstr_desc.dsc$w_length = sig_array[1];
      retstr_desc.dsc$a_pointer = sig_array[2];
      str$append( outstr_desc_ptr, &retstr_desc );      /* append retstr to end of retcode */
   }
}

/*---------------------------------------------------------------------------

        .SBTTL  EDX_SIGNAL

 Functional Description:
        This routine prints message text to the message buffer.  The routine
        input is modeled after LIB$SIGNAL.  (See description of LIB$SIGNAL)

 Calling Sequence:
                 edx_signal(nargs,sig_array);

 Argument inputs:
        nargs     - Number of array elements in sig_array (by value)
        sig_array - sig_array[0] = condition-value1
                              .  = [number-of-args-for-condition-value1]
                              .  = [FAO-arg1]
                              .  = [,...]
                              .  = [FAO-argn]
                              .  = [condition-value2]
                              .  = [number-of-args-for-condition-value2]
                              .  = [FAO-arg2]
                              .  = [,...]
                              .  = [FAO-argn]
                              .  = [etc.]
    condition-value1
       The condition to be signaled.  (by value)
    FAO-arg1 (unspecified.  Values sent directly to FAO.)
        etc.

 Outline:
    1.  Check severity of message to signal.  If FATAL then use full
        message format (facility, identification, severity, text). Otherwise
        obtain current TPU message flags by calling ourselves using
        TPU$EXECUTE_COMMAND.
           The TPU command executed is:
           EDTN$X_DUMMY := CALL_USER( ^x00020000 + GET_INFO(SYSTEM,"message_flags"), "") )
           which parses to:
           EDTN$X_DUMMY := CALL_USER( ^x0002000F, "")
           where 'F' is the current setting of the message flags (0-F hex).
           This value gets stored in msgflgs.

    2.  Check TPU VERSION & UPDATE.  If 2.4 or above then UPDATE(MESSAGE_WINDOW).
        This fixes bug in VMS 5.3 so next message will appear properly

    LOOP
    3.  Call sys$getmsg to get message text
    4.  Call sys$fao to process message text
    5.  Call tpu$message to write message text to tpu message_buffer.
    ENDLOOP (until sig_array exhausted)
;-----------------------------------------------------------------------------*/

void edx_signal( int nargs, int *sig_array )
{
   int status;
   int arg_count = 0;
   char fao_output[BUFLEN];
   char message_string[BUFLEN];
   $DESCRIPTOR(fao_output_desc, fao_output);
   $DESCRIPTOR(message_string_desc, message_string);
   $CONST_DESCRIP(msgflgcmd_desc, "EDTN$X_DUMMY:=CALL_USER(131072+GET_INFO(SYSTEM,'MESSAGE_FLAGS'),'')" );
   $CONST_DESCRIP(updmsgcmd_desc, "IF GET_INFO(SYSTEM,'DISPLAY') THEN UPDATE(MESSAGE_WINDOW) ENDIF;" );

   /* get current value of message flags */
   tpu$execute_command(&msgflgcmd_desc);

   /* Now do special check for VMS 5.3 bug.  If running VAXTPU 2.4 or above,
      then make call to update(message_window) BEFORE sending message to message buffer.
      Someday when they fix this bug we'll branch if VAXTPU above where bug is fixed. */
   if (TPU$GL_VERSION == 2 && TPU$GL_UPDATE &gt;= 4)
         tpu$execute_command(&updmsgcmd_desc);

   /* Check severity of primary message.  Ring bell if not good. */
   if ( !(sig_array[0] & STS$M_SUCCESS) )                       /* if WARNING, ERROR, or FATAL (not SUCCESS or INFORMATIONAL) */
   {
      if ( !((sig_array[0] & STS$M_SEVERITY) == STS$K_WARNING)) /* no bell for warning */
         lib$put_output(&bell_desc);                            /* Ring terminal bell for ERROR or FATAL */

      /* If message is fatal, use full message flags */
      if ( (sig_array[0] & STS$M_SEVERITY) == STS$K_SEVERE)
         msgflgs = 15;                                  /* Use full message format */
   }

   /* BEGIN MAIN LOOP */
   do
   {
      message_string_desc.dsc$w_length = BUFLEN;        /* initialize buffer length in message string */
      fao_output_desc.dsc$w_length = BUFLEN;            /* initialize buffer length in for FAO output string */

      /* Get message text */
      sys$getmsg(sig_array[arg_count],                  /* msgid */
                 &message_string_desc.dsc$w_length,     /* msglen (by reference) */
                 &message_string_desc,                  /* bufadr (by descriptor) */
                 msgflgs,                               /* flags (by value) */
                 0 );                                   /* outadr */

     /* Check for FAO arguments */
     ++arg_count;                                       /* sig_array[arg_count] = [number-of-args-for-condition-value1] */
     if (arg_count &lt; nargs)                             /* Number of arguments used &lt; total number of arguments */
     {
         sys$faol( &message_string_desc,                /* ctrstr */
                  &fao_output_desc.dsc$w_length,        /* outlen */
                  &fao_output_desc,                     /* outbuf */
                  &sig_array[arg_count+1] );            /* prmlst *//* sig_array[arg_count] = FAO argument #1 */
         tpu$message( &fao_output_desc );
     }
     else
     {
        /* Print out last (or only) message */
        tpu$message( &message_string_desc );
        return;
     }
     /* See if there's another message to do */
     arg_count += sig_array[arg_count] + 1;
  }
  while(arg_count &lt; nargs);
  return;
}
/*---------------------------------------------------------------------------

        .SBTTL  CONDITION HANDLER

 Functional Description:
        This routine handles unexpected errors that are signaled.
        The error is printed to the TPU message_buffer.
        If the error is severe, an attempt is made to return to TPU,
        otherwise the error is non-fatal and an attempt is made to continue.

        A signal of any type including Informational (-I-) is not
        resignaled because the TPU CALL_USER routine establishes it's own
        condition handler that will be called next if our handler does not
        handle the condition, and the TPU handler calls LIB$SIG_TO_RET if
        the signaled condition does not have a facility value of TPU.

        Therefore we never return with SS$_RESIGNAL (unless unwinding).

 Calling Sequence:
        This routine is called by the operating system whenever a
        condition is signaled.  A memory access violation or a
        divide by zero error for example would be signaled.

 Argument inputs:

       sigargs array (both VAX/VMS & OpenVMS Alpha AXP):
        -----------------------------------------
        |             vector length             |  sig_array[0]
        -----------------------------------------
        |            condition value            |  sig_array[1]
        -----------------------------------------
        |Additional arguments as required (or none)
        |                   .                   |
        |                   .                   |
        -----------------------------------------
        |                  PC                   |
        -----------------------------------------
        |                  PSL                  |
        -----------------------------------------

Notes:  Even though many conditions do not use PC & PSL in their FAO
        arguments, we always send them to EDX_SIGNAL because it's
        easier that way.  When EDX_SIGNAL calls FAO, FAO will use
        only what it needs.
*/

unsigned long int handler(unsigned int *sigargs, unsigned int *mechargs)
{
   int i;
   int sig_array[16];   /* Let's hope no condition uses more than 12 FAO arguments */

   if (sigargs[1] == SS$_UNWIND) return(0);

   if (sigargs[1] & STS$K_SEVERE)               /* check for fatal status */
   {
      sig_array[0] = EDX__UNEXPERR;             /* Unexpected Fatal Error */
      sig_array[1] = 0;                         /* 0 FAO arguments for EDX__UNEXPERR */
      sig_array[2] = sigargs[1];                /* condition value */
      sig_array[3] = sigargs[0]-1;              /* number of FAO arguments for condition value */
      for( i = 4; i &lt; sigargs[0]+3 && i &lt; 16; ++i )
         sig_array[i] = sigargs[i-2];           /* the FAO arguments */
     edx_signal( i, sig_array);                 /* print the error messages */
     i = sys$unwind(0,0);                       /* unwind stack */
   }
   else                                         /* handle unexpected fatal errors */
   {
      sig_array[0] = sigargs[1];                /* condition value */
      sig_array[1] = sigargs[0]-1;              /* number of FAO arguments for condition value */
      for( i = 2; i &lt; sigargs[0]+1 && i &lt; 16; ++i )
         sig_array[i] = sigargs[i];             /* the FAO arguments */
     edx_signal( i, sig_array);                 /* print the error messages */
     return(SS$_CONTINUE);                      /* return from exception */
   }

}
/*-------------------------------------------------------------------------*/

#if DEBUG
/* SOME TESTS OF THE ERROR HANDLER */
void test_handler()
{
   static readonly int a = 1;
   int incodel;
   int d = 0;
   int e,u,v;

   incodel = (*incode_ptr & 0x0000FFFF);
   switch (incodel)                     /* Case entry point to jump to */
   {
      case 1:  test_handler_sub(&a);  break;    /* Generate access violation */
      case 2:  e = 1/d;  break;                 /* Divide by zero */
      case 3:  v = TPU$GL_VERSION;
               fmtoutstr( 1, &v );
               break;
      case 4:  u = TPU$GL_UPDATE;
               fmtoutstr( 1, &u );
               break;
      default: edx_signal(1, byref(EDX__UNKNCODE) ); break;
   }
}
void test_handler_sub(int *m)
{
   *m = 0;      /* Test handler by generating access violation.  Modify readonly variable */
}
#endif
/*

===========================================================================
        DIRECTORY
===========================================================================
        .SUBTITLE EDX_DIRECTORY

 Functional Description:
        Displays a directory listing

 Calling Sequence:
        edx_directory();

 Argument inputs:
        incode_ptr = Points to incode.  Low word of incode is code
                     describing where to reenter (which case to go to).
        instr_desc = Directory command (by descriptor).  The directory
                     command is of the form:
                        DIRECTORY [/SIZE] [/DATE] [dir-spec]

 Outputs:
        OUTSTR  = Line to place in DIR_BUFFER (by descriptor)
        RETCODE = Code to use for consecutive calls
        (note: OUTSTR and RETCODE are placed together in the outstr_desc
         string returned by FMTOUTSTR routine.  RETCODE is stored in first
         9 characters of the returned output string.)

 Comments:
        For regular directory listing, filenames are placed 4 across
        staring at column offsets 0,20,40,60.

        For /SIZE or /DATE qualifiers, filenames are placed at column offset 0.
        Then if col is greater than 18 line is written and new line started
        Error starts at column offset 19 if there is one
        else /SIZE starts at offset 19 if specified
         and /DATE starts at offset 29 if specified

        COL is used as both a length indicator and a column offset indicator.
        The value of COL is the number of characters in OUTLINE.  When the
        value of COL is added to the base address of OUTLINE, the result is
        the address to start adding text to when appending text out OUTLINE.

 Outline:
        1.  Entry code is cased for consecutive reentries
dstrt:  2.  On initial entry
            A.  The default setting for /SIZE and /DATE in Dirflgs is
                initialized by checking for a defined symbol DIR.
                For example, the user may have defined
                  DIR :== DIRECTORY/SIZE/DATE
                in which case we'll give him the /SIZE /DATE by default
                unless he specifically negates it with /NOSIZE /NODATE.
            B.  The EDX directory command is parsed for /SIZE, /DATE,
                and the dir-spec.
            C.  a.  XABDAT block initialized
                b.  XABFHC block initialized
                c.  NAM block initialized
                d.  FAB block initialized.  Dir-spec placed into FAB.
                e.  Variables are initialized
            D.  Set DIRFLGS according to /SIZE and /DATE qualifiers
parse:  3.  $PARSE FAB to prepare for wildcard operations
srchlp: 4.  $SEARCH FAB for next filename
            A.  If 'No more files' or 'File Not Found' or other error,
                exit with code.
            B.  IF this is the first call, return first with the expanded
                string for the window status line.
                ELSE IF the returned directory root has changed then
                print the contents of outline and then
proot:          print the new directory heading.
nxttab: 5.  Add new file to outline.
            A.  Move COL offset pointer into outline to next tab stop (0,20,40,60)
                Print outline if line full.
adfile:     B.  Add filename to outline
        7.  Check DIRFLGS for qualifiers like /SIZE and /DATE
            A.  If no qualifiers present then goto srchlp:
            B.  Print outline if filename too long (overflows into SIZE field)
getatr:     C.  Get file attributes
            D.  Add size info if requested
            E.  Add date info if requested
            F.  Print outine
            G.  Goto srchlp:
        8.  Repeat until exit by step 4.

 Description:
        The TPU editor calls us with an initial code of START.  We return to
        the TPU editor with an output string and a code telling the editor what
        to do with the output string.  Usually the TPU editor is to print the
        output string to the dir_buffer, occasionally it must add a blank line
        or two.  It then calls us back passing to us the value of code we gave
        it.  The value of code tells us where to jump back to.  The TPU editor
        continues to call us until we pass it the NMF_ERR code.

Coding notes:
        The logic would flow smoothly if everytime we wanted to print a line
        we just called a subroutine to do so.  However, in order to print a
        line here we must return to the caller.  Thus we need to use goto
        statements and labels so we can jump back in to where we left off
        so the logic still flows smoothly.

        It's actually possible to get a file's size and date from the file
        without actually opening the file by using the RMS $DISPLAY call
        instead.  We use $OPEN here to better emulate the $ DIR/SIZE/DATE
        command which will give us an error if the file is currently open
        or if we don't have enough privilege to open the file.

---------------------------------------------------------------------------*/
#define DIR_DSTRT     1    /* first call */
#define DIR_SRCHLP    2    /* TPU prints outline and calls again */
#define DIR_PROOT     3    /* TPU prints outline followed by one blank lines and calls us again */
#define DIR_NXTTAB    4    /* TPU prints outline followed by one blank line and calls us again */
#define DIR_ADFILE    5    /* TPU prints outline and calls us again */
#define DIR_GETATR    6    /* TPU prints outline and calls us again */
#define DIR_RMS_ERR   7    /* Return code.  We print error message.  TPU exits. */
#define DIR_FNF_ERR   8    /* Return code.  TPU prints 'no files found' and exits */
#define DIR_NMF_ERR   9    /* Return code.  TPU prints outline and exits */
#define DIR_PSUBTOT  10    /* TPU prints outline followed by one blank line and calls us again */
#define DIR_SUMMARY  11    /* TPU prints outline followed by one blank line and calls us again */
#define DIR_GTTL     12    /* TPU prints outline containing grand totals and quits.  All done. */

/* dir flags */
#define DIRFLGS_GET_SIZE  1
#define DIRFLGS_GET_DATE  2

/*
;column offset positions & constants
;....v....1....v....2....v....3....v....4....v....5....v....6....v....7....v....
;FILENAMEABC.EFG;21 00000000  25-JUL-1988 17:14
*/
#define DIR_DATCOL  29          /* offset to start date */
#define DIR_SIZCOL  19          /* offset to start size */
#define DIR_SIZLEN   8          /* length of size field */
#define DIR_DATLEN  17          /* length of date field */

/* MACROS */
#define PRINT_OUTLINE(reentry_code) {\
      sig_array[0] = reentry_code;\
      sig_array[1] = col;\
      sig_array[2] = outline;\
      fmtoutstr( 3, &sig_array );\
      memset( outline, ' ', BUFLEN );\
      col = 0;\
      return;\
}
/*---------------------------------------------------------------------------*/

void edx_directory()
{
   unsigned short retlen;
   int status;
   int incodel;
   int nxtcol;
   int file_size;
   int sig_array[3];            /* used by PRINT_OUTLINE macro */
   static int total_directories;/* Total number of directories */
   static int total_files;      /* Total number of files for this directory */
   static int total_blocks;     /* Total number of blocks for this directory (when using /SIZE) */
   static int grand_total_files;   /* Grand total number of files for all directories */
   static int grand_total_blocks;  /* Grand total number of blocks for all directories */
   static int col;              /* column offset.  (MBR) = Must Be Retained between calls */
   static int dirflgs;          /* indicates if /SIZE or /DATE specified. (MBR) */
   static int rootlen;          /* length of node::device:[directory]  (MBR) */
   static char input_file[NAM$C_MAXRSS];
   static char result_file[NAM$C_MAXRSS];
   static char expanded_file[NAM$C_MAXRSS];
   static char root[NAM$C_MAXRSS];
   static char outline[BUFLEN]; /* used by PRINT_OUTLINE macro */
   static struct FAB dirfab;
   static struct NAM dirnam;
   static struct XABFHC dirxabfhc;
   static struct XABDAT dirxabdat;
   struct dsc$descriptor_s outline_desc = { BUFLEN, DSC$K_DTYPE_T, DSC$K_CLASS_S, outline };
   struct dsc$descriptor_s outlnsize_desc = { DIR_SIZLEN, DSC$K_DTYPE_T, DSC$K_CLASS_S, &outline[DIR_SIZCOL] };
   struct dsc$descriptor_s outlndate_desc = { DIR_DATLEN, DSC$K_DTYPE_T, DSC$K_CLASS_S, &outline[DIR_DATCOL] };
   struct dsc$descriptor_s errmsg_desc;
   $DESCRIPTOR(input_file_desc,input_file);
   $CONST_DESCRIP(fao_total, "Total of !SL file!%S.");
   $CONST_DESCRIP(fao_totsize, "Total of !SL file!%S, !SL block!%S.");
   $CONST_DESCRIP(fao_gtotal, "Grand total of !UL directories, !UL file!%S.");
   $CONST_DESCRIP(fao_gtotsize, "Grand total of !UL directories, !UL file!%S, !UL block!%S.");
   $CONST_DESCRIP(dirspec_desc,"DIRSPEC");

/* Entry code is cased for reentry point */
   incodel = (*incode_ptr & 0x0000FFFF);
   switch (incodel)
   {
      case DIR_DSTRT:   goto dstrt;                     /* Initial entry */
      case DIR_SRCHLP:  goto srchlp;                    /* TPU printed outline and called us again */
      case DIR_PSUBTOT: goto psubtot;                   /* TPU printed outline followed by one blank line and called us again */
      case DIR_PROOT:   goto proot;                     /* TPU printed outline followed by one blank line and called us again */
      case DIR_NXTTAB:  goto nxttab;                    /* TPU printed outline followed by one blank line and called us again  */
      case DIR_ADFILE:  goto adfile;                    /* TPU printed outline and called us again */
      case DIR_GETATR:  goto getatr;                    /* TPU printed outline and called us again */
      case DIR_SUMMARY: goto summary;                   /* TPU printed outline containing last filenames, followed by one blank line, and called us again to get totals and grand totals */
      case DIR_GTTL:    goto gttl;                      /* TPU printed outline containing totals for this directory, followed by one blank line, and called us again to get grand totals */
      default:  edx_signal(1, byref(EDX__UNKNCODE) ); return;
   }

dstrt:
   getdefdirflgs(&dirflgs);                             /* Dirflgs is initialized */
   status = cli$dcl_parse(&instr_desc,&edx_commands);   /* The directory command is parsed using CLI$DCL_PARSE for /SIZE, /DATE, and dir-spec. */
   if ( !(status & STS$M_SUCCESS) ) return;             /* return on error.  CLI$DCL_PARSE signaled error and our condition handler printed the error. */

   /* INITIALIZE XABDAT BLOCK */
   dirxabdat = cc$rms_xabdat;                           /* initialize XABDAT */

   /* INITIALIZE XABFHC BLOCK */
   dirxabfhc = cc$rms_xabfhc;                           /* initialize XABFHC */
   dirxabfhc.xab$l_nxt = &dirxabdat;                    /* Address of next XAB (XABDAT) */

   /* INITIALIZE NAM BLOCK */
   dirnam = cc$rms_nam;                                 /* initialize NAM */
   dirnam.nam$b_rss = NAM$C_MAXRSS;                     /* Resultant file name string size */
   dirnam.nam$l_rsa = (char *) result_file;             /* Resultant file name string address */
   dirnam.nam$b_ess = NAM$C_MAXRSS;                     /* Expanded file name string size */
   dirnam.nam$l_esa = (char *) expanded_file;           /* Expanded file name string address */

   /* INITIALIZE FAB BLOCK */
   dirfab = cc$rms_fab;                                 /* initialize FAB */
   dirfab.fab$l_nam = &dirnam;                          /* NAM block address */
   dirfab.fab$l_xab = &dirxabfhc;                       /* XAB block address (XABFHC)    */
   dirfab.fab$l_fop = FAB$M_NAM;                        /* FAB Options = use NAM block   */
   dirfab.fab$l_dna = (char *) "*.*;*";                 /* Default file name of "*.*;*"  */
   dirfab.fab$b_dns = 5;                                /* Default file name length      */
   dirfab.fab$l_fna = (char *) input_file;              /* Address of input string containing dir-spec */
   dirfab.fab$b_fac = FAB$M_GET;                        /* File Access options = GET */
   /* Allow read/write sharing in case we have to open the file to get the file attributes */
   dirfab.fab$b_shr = FAB$M_SHRGET | FAB$M_SHRUPD | FAB$M_SHRPUT | FAB$M_SHRDEL;

   /* INITIALIZE VARIABLES */
   memset( outline, ' ', BUFLEN );                      /* clear outline          */
   total_directories = 0;                               /* start with 0 directories */
   col = 0;                                             /* Set column offset := 0 */
   grand_total_files = 0;                               /* Grand total number of files for all directories */
   grand_total_blocks = 0;                              /* Grand total number of blocks for all directories */

   /* GET DIR-SPEC INTO FAB BLOCK BY CALLING CLI$GET_VALUE */
   cli$get_value( &dirspec_desc, &input_file_desc, &retlen);    /* Dir-spec placed into FAB by calling cli$get_value */
   dirfab.fab$b_fns = retlen;

   status = cli$present( s_descrip("SIZE") );           /* check for /SIZE present in command line */
   if ( status == CLI$_PRESENT ) dirflgs |= DIRFLGS_GET_SIZE;
   else if ( status == CLI$_NEGATED ) dirflgs &= ~DIRFLGS_GET_SIZE;

   status = cli$present( s_descrip("DATE") );           /* check for /DATE present in command line */
   if ( status == CLI$_PRESENT ) dirflgs |= DIRFLGS_GET_DATE;
   else if ( status == CLI$_NEGATED ) dirflgs &= ~DIRFLGS_GET_DATE;

   /* PARSE THE DIR-SPEC */
parse:
   status = sys$parse( &dirfab );                       /* $PARSE FAB to prepare for wildcard operations */
   if ( !(status & STS$M_SUCCESS) )
   {
      edx_signal(1,&status);                            /* signal error (could be Directory Not Found, Invalid Device Name, etc) */
      PRINT_OUTLINE(DIR_RMS_ERR)                        /* and return error */
   }

/* SEARCH LOOP */
srchlp:
   status = sys$search( &dirfab );                      /* $SEARCH FAB for next filename */
   switch (status)
   {
      case RMS$_NORMAL:
         break;                                         /* continue if normal */
      case RMS$_FNF:                                    /* File Not Found           */
         PRINT_OUTLINE(DIR_FNF_ERR)                     /* return error */
      case RMS$_NMF:                                    /* 'No More Files'          */
         PRINT_OUTLINE(DIR_SUMMARY)                     /* Print last line of files followed by summary of total and grand_total and then quit */
      default:
         edx_signal(1,&status);                         /* Wasn't any of the expected errors so signal error and return */
         PRINT_OUTLINE(DIR_NMF_ERR)                     /* return error */
   }

   /* Print new directory heading if needed     */
   /* WE HAVE A NEW FILENAME TO PROCESS.        */
   /* PRINT OUT NEW DIRECTORY HEADING IF NEEDED */
   if (total_directories == 0)
   {
      /* Print directory specification */
      total_directories = 1;
      col = dirnam.nam$b_esl;
      PRINT_OUTLINE(DIR_PROOT)                          /* If this is the first call, return first with the expanded string for the window status line.  Then go print the directory heading. */
   }
   if ( strncmp( root, result_file, rootlen ) == 0 )
      goto nxttab;                                      /* Branch if root is still same */
   ++total_directories;                                 /* We have a new directory heading */
   PRINT_OUTLINE(DIR_PSUBTOT)                           /* print last line of filenames followed by totals for this directory followed by new directory heading */

psubtot:
   if (total_directories &gt; 1)                           /* IF MULTIPLE_ROOTS THEN FIRST PRINT TOTAL FOR PREVIOUS ROOT */
      if ( (dirflgs & DIRFLGS_GET_SIZE) == 0 )
         sys$fao( &fao_total,                           /* "Total of !SL files." */
                  &col,
                  &outline_desc,
                  total_files );
      else
         sys$fao( &fao_totsize,                         /* "Total of !SL files, !SL blocks." */
                  &col,
                  &outline_desc,
                  total_files,
                  total_blocks );
      PRINT_OUTLINE(DIR_PROOT)                          /* print totals for this directory followed by new directory heading */

proot:
   /* PRINT NEW DIRECTORY HEADING */
   /* FILENAME IS OF FORM NODE::DEV:[DIR]NAME.TYPE;VER */
   total_files = 0;                                     /* reset number of files to 0 */
   total_blocks = 0;                                    /* reset total size to 0 */
   rootlen = dirnam.nam$b_node
           + dirnam.nam$b_dev
           + dirnam.nam$b_dir;                          /* Calculate length of node::dev[dir] */
   memcpy(root,result_file,rootlen);                    /* root = result_name(1:rootlen); */
   memcpy(outline,"Directory ",10);                     /* outline = "Directory " (10 chars) */
   memcpy(&outline[10],result_file,rootlen);            /* outline = "Directory {dirspec}" */
   col = 10 + rootlen;                                  /* Length of outline = "Directory {dirspec}" */
   PRINT_OUTLINE(DIR_NXTTAB)                            /* print outline and go to nxttab: */

nxttab:
   /* Add new file to outline. */
   /* Move COL offset pointer into outline to next tab stop (0,20,40,60) */
   /* Print outline if line full. */
   /* MOVE TO NEXT TAB STOP (0,20,40,60) */
   if (col != 0)                                        /* If COL = 0 then go add next filename */
   {
      if      (col &lt; 20) nxtcol = 20;                   /* Advance COL to 20 */
      else if (col &lt; 40) nxtcol = 40;                   /* Advance COL to 40 */
      else if (col &lt; 60) nxtcol = 60;                   /* Advance COL to 60 */
      else nxtcol = 80;                                 /* (insures we will print this line below */

      if (nxtcol + dirnam.nam$b_name
                 + dirnam.nam$b_type
                 + dirnam.nam$b_ver &gt;= 80)
      {                                                 /* print line.  Another filename won't fit on this line */
         PRINT_OUTLINE(DIR_ADFILE)                      /* print outline and go to adfile: */
      }
      col = nxtcol;                                     /* Advance to next column and add next filename */
   }/* if (col != 0) */

adfile:
   /* Add filename to outline */
   /* ADD NEW FILENAME TO OUTLINE */
   memcpy(&outline[col],dirnam.nam$l_name,
      dirnam.nam$b_name + dirnam.nam$b_type + dirnam.nam$b_ver);
   ++total_files;       /* Increment number of files in this directory listing */
   ++grand_total_files;

   col = col + dirnam.nam$b_name + dirnam.nam$b_type + dirnam.nam$b_ver;

   /* Check DIRFLGS for qualifiers like /SIZE and /DATE */
   /* PRINT OUTLINE IF FILENAME TOO LONG */
   if (dirflgs == 0) goto srchlp;                       /* no qualifiers present, goto loop */
   if (col &gt;= DIR_SIZCOL)                               /* Check column offset.  Check for extra-long filename */
      PRINT_OUTLINE(DIR_GETATR)                         /* print outline and go to adfile: */

getatr:
   /* /SIZE AND/OR /DATE PRESENT */
   status = sys$open( &dirfab );                        /* Get file attributes */
   if ( !(status & STS$M_SUCCESS) )
   {
      short int errmsg_len;
      errmsg_desc.dsc$w_length = NAM$C_MAXRSS - DIR_SIZCOL;
      errmsg_desc.dsc$b_class = DSC$K_CLASS_S;
      errmsg_desc.dsc$b_dtype = DSC$K_DTYPE_T;
      errmsg_desc.dsc$a_pointer = &outline + DIR_SIZCOL;
      sys$getmsg( status, &errmsg_len, &errmsg_desc, 1, 0 );
      col = DIR_SIZCOL + errmsg_len;
      PRINT_OUTLINE(DIR_SRCHLP)                 /* print outline containing error message and go to srchlp: */
   }

   /* File successfully opened.  Close file and process qualifiers. */
   sys$close( &dirfab );

   if ( (dirflgs & DIRFLGS_GET_SIZE) != 0 )
   {
      file_size = dirxabfhc.xab$l_ebk;
      if ( dirxabfhc.xab$w_ffb == 0 ) --file_size;      /* If first free byte = 0 then don't count last block */
      total_blocks += file_size;                        /* add to total */
      grand_total_blocks += file_size;                  /* add to total */
      sys$fao( s_descrip("!8UL"),                       /* ctrstr (DIR_SIZLEN=8) */
               0,                                       /* outlen */
               &outlnsize_desc,                         /* outbuf (outline starting at offset for size) */
               file_size );                             /* P1 */
      col = DIR_SIZCOL + DIR_SIZLEN;
   }

   if ( (dirflgs & DIRFLGS_GET_DATE) != 0 )
   {
      sys$asctim( 0,                                    /* timlen */
                  &outlndate_desc,                      /* timbuf */
                  &dirxabdat.xab$q_cdt,                 /* timadr (file creation date) */
                  0 );                                  /* cvtflg */
      col = DIR_DATCOL + DIR_DATLEN;                    /* Set col pointer to end of date */
   }
   PRINT_OUTLINE(DIR_SRCHLP)                            /* print outline and go to srchlp: */

summary:
   if ( (dirflgs & DIRFLGS_GET_SIZE) == 0 )
      sys$fao( &fao_total,                              /* "Total of !SL files, !SL blocks." */
               &col,
               &outline_desc,
               total_files );
   else
      sys$fao( &fao_totsize,                            /* "Total of !SL files." */
               &col,
               &outline_desc,
               total_files,
               total_blocks );
   if (total_directories &gt; 1)                           /* IF MULTIPLE_ROOTS THEN FIRST PRINT TOTAL FOR PREVIOUS ROOT */
      PRINT_OUTLINE(DIR_GTTL)                           /* print totals for this directory followed by one blank line.  Call again for grand totals. */
   else
      PRINT_OUTLINE(DIR_NMF_ERR);                       /* print totals and quit.  We're done. */

gttl:
   if ( (dirflgs & DIRFLGS_GET_SIZE) == 0 )
      sys$fao( &fao_gtotal,                             /* "Grand total of !UL directories, !UL files." */
               &col,
               &outline_desc,
               total_directories,
               grand_total_files );
   else
      sys$fao( &fao_gtotsize,                           /* "Grand total of !UL directories, !UL files, !UL blocks." */
               &col,
               &outline_desc,
               total_directories,
               grand_total_files,
               grand_total_blocks );
   PRINT_OUTLINE(DIR_NMF_ERR);                  /* print grand totals and quit.  We're done. */
}

/*---------------------------------------------------------------------------

        .SUBTITLE GETDEFDIRFLGS

 Functional Description:
        Attempts to translate DCL symbol 'DIR' and determine if /SIZE
        or /DATE qualifiers are present.

 Calling Sequence:
        getdefdirflgs(dirflgs);

 Arguments:
        dirflgs = int to put dirflgs in, (by reference).

 Outputs:
        dirflgs = DIRFLGS_GET_SIZE bit set if /SIZE is present
                  DIRFLGS_GET_DATE bit set if /DATE is present

 Outline:
        1.  LIB$GET_SYMBOL is called to obtain the symbol translation
        2.  CLI$PRESENT is called to determine if qualifiers are present
            and dirflgs is set accordingly.

----------------------------------------------------------------------------*/

unsigned long int null_handler(unsigned int *sigargs, unsigned int *mechargs)
{
   return(SS$_CONTINUE);        /* dismiss all exceptions */
}

void getdefdirflgs(int *dirflgs_ptr)
{
   int status;
   int tblind;
   char buffer[BUFLEN];
   $DESCRIPTOR(buffer_desc,buffer);

   VAXC$ESTABLISH(null_handler);        /* Suppress all signals from CLI$DCL_PARSE and CLI$PRESENT */
   *dirflgs_ptr = 0;                    /* Initialize dirflgs */

   if ( lib$get_symbol( s_descrip("DIR"),
                        &buffer_desc,
                        &buffer_desc.dsc$w_length,
                        &tblind )                       /* Translate symbol */
        & STS$M_SUCCESS )                               /* if translation succeeded */
   {
      status = cli$dcl_parse( &buffer_desc, &edx_commands );    /* Parse input string */
      if (cli$present(s_descrip("SIZE")) == CLI$_PRESENT)               /* if /SIZE was present */
         *dirflgs_ptr |= DIRFLGS_GET_SIZE;                              /* then set bit for /DATE */
      if (cli$present(s_descrip("DATE")) == CLI$_PRESENT)               /* if /DATE was present */
         *dirflgs_ptr |= DIRFLGS_GET_DATE;                              /* then set bit for /DATE */
   }
}
/*

==============================================================================
        EDX SPELL
==============================================================================

EDX DICTIONARY LEXICAL DATABASE: Source
The EDX spelling dictionary now contains over 90,000 words, all of which
are believed to be correctly spelled.  Every effort has gone into
verifying the integrity of the EDX spelling dictionary.  All of the
words in the lexical database file DICTIONARY.LEX have been cross
checked against three separate spelling checkers commercially available
for home personal computers.  These are:

        The Random House Proofreader version 2.00
        Wang Electronic Publishing, Inc.
        P.O. Box 367
        Tijeras, NM 87059

        WordPerfect Speller from WordPerfect version 5.1
        WordPerfect Corporation
        1555 N. Technology Way
        Orem, Utah 84057

        Spell Check module of Multi-Edit Professional version 6.0
        American Cybernetics
        1830 W. University Dr. #112
        Tempe, AZ 85281

In the course of cross checking the dictionaries it was found that
WordPerfect 5.1 contained 90 words which were either misspelled (such as
"postoffice") or so obscure that they were not found in any of the
unabridged dictionaries our local library had nor were they found in any
other reference book we could think of to try.  WordPerfect Corporation
has been notified of these words.  It was also found that Multi-Edit
Professional version 6.0 contained 365 words which were definitely
misspellings.  American Cybernetics has been notified of these words.
In addition it was found that the popular Vassar spelling checker
available on DECUS tapes contains about 20,000 misspelled words.

The words in the common word list file COMMONWORDS.LEX came from "The
American Heritage Word Frequency Book, copyright 1971 by American
Heritage Publishing Co, Inc.".


EDX DICTIONARY:  Layout
It is possible to examine the EDX dictionary file EDX_DICTIONARY.DAT
with the EDX editor.  Invoke the EDX editor and read in the file as
you would any other file.  Each line of the file represents one 512 byte
block of the file.  The line number corresponds to the Virtual Block
Number (VBN) of the block.

DICTIONARY FILE LAYOUT:
To optimize performance we use direct disk block transfers which
bypasses the concept of file records.  The disk block size is 512
bytes.  Once the file is opened for block I/O (by specifying BIO in
the FAC field of the FAB), the disk blocks can be randomly accessed by
specifying the Virtual Block Number (VBN) of the block you wish to
read or write.  Each section of the database begins on a block
boundary.

DICTIONARY FILE LAYOUT:
--------------------
|   HEADER BLOCK   |    (VBN 1)
|                  |
--------------------
| LEXICAL DATABASE |    (VBN 2)
|                  |
|                  |
|                  |
|                  |
|                  |
--------------------
|      INDEX       |    (around VBN 2000)
|                  |
|                  |
--------------------
| COMMON WORD LIST |    (around VBN 2008)
|                  |
--------------------

HEADER
Virtual Block number 1 (VBN 1) contains information about the rest of
the database, where it's located in the file and how long each section
is.  The figure below describes the header block in detail.

FILE HEADER (VBN 1)
--------------------------------------------------
| 't' | 'c' | 'i' | 'd' | 'X' | 'D' | 'E' |VERSNO| 00   header
--------------------------------------------------
|                    LEXVBN                      | 08   Virtual Block Number of disk block where lexical database begins
--------------------------------------------------
|                    LEXBLN                      | 10   Length in blocks of lexical database section
--------------------------------------------------
|                    INDVBN                      | 18   Virtual Block Number of disk block where index database begins
--------------------------------------------------
|                    INDLEN                      | 20   Length in bytes of index database
--------------------------------------------------
|                    INDSWD                      | 28   Size of each index word
--------------------------------------------------
|                    DICPLN                      | 30   Dictionary Page length.  Number of bytes (of lexical database) between index guide words
--------------------------------------------------
|                    CWDVBN                      | 38   Virtual Block Number of disk block where common word list begins
--------------------------------------------------
|                    CWDLEN                      | 40   Length in bytes of common word list
--------------------------------------------------
|                    CWDMLN                      | 48   Length in bytes of longest word in common word list
--------------------------------------------------


LEXICAL DATABASE: Format
The Lexical Database portion contains the actual list of ~90,000 words,
hopefully some of which are correctly spelled.  The words must appear in
alphabetical order, with a single length-byte preceding each word, the
very last word in the lexical database stream is followed by a NULL
byte.  All alphabetic characters must be lowercase.  The contiuous
stream of characters looks like this:

  ".A.AARDVARK.AARDVARKS.AARON.AAVC.ABACK.ABACTERIAL.ABACUS....
   1 8        9         5     4    5     10         6

Here the (.) indicates a byte whos ASCII value is given below it.
A section of this word list will later be searched for a match to an
unknown word preceded by a length-byte of the words length, such as:

  search list for:  ".AARON"
                     5

INDEXING CONCEPTS
Page Length:
Conceptually, our lexical database section is broken into fixed length
pages.  The value DICPLN stored in the header block indicates the page
length (in bytes) used in the dictionary.

Guide Words
In a regular paper dictionary there is at the top of each page a guide
word which indicates the first word for that page.  In our dictionary we
use fixed length guide words, saving only the first N letters of the
first full word of each lexical database page.  (Note that characters of
a word may lie across a page boundary, we only accept the first full
word after passing over those trailing characters of the previous word which
started on the previous page.)  The value of N is the value of INDSWD stored
in the header block.  If the guide word is less than N characters long it is
blank padded to N characters.

INDEX
The index section is a contiguous stream of the ordered fixed length
guide words.  This is the information which helps us quickly zero in on
where in the lexical database to search for a given word.

An example may explain best:
Assuming an index word size (INDSWD) of 4, and a page length DICPLN of
1024 bytes the first 4 pages of our dictionary would look like the
following:

 A                                                     (guide word for page 1 is "A   ")
 .A.AARDVARK.AARDVARKS.AARON.AAVC.ABACK.ABACTERIAL...  (page 1 consists of first 2 blocks (1024 bytes) of the lexical database.
 ABJE                                                  (guide word for page 2 is first 4 letters of word ABJECT)
 OTICALLY.ABJECT.ABJECTION.ABJECTLY.ABJECTNESS...      (page 2 consists of next 2 blocks (1024 bytes) of the lexical database)
 ABRE                                                  (guide word for page 3 is first 4 letters of word ABREAST)
 REACTION.ABREAST.ABRIDGE.ABRIDGED.ABRIDGEMENT...      (page 3 consists of next 2 blocks (1024 bytes) of lexical database)
 ABST                                                  (guide word for page 4 consists of first 4 letters of word ABSTRACTION)
 STRACTING.ABSTRACTION.ABSTRACTIONAL.ABSTRACTIONISM... (page 4 consists of next 2 blocks (1024 bytes) of lexical database)

The index contains the first 4 characters of the first full word of
every page in the lexical database.  The 4 character index words are
packed together into a continuous stream of characters.  For our above
example, the index would look like:

     guide word:  "A   ABJEABREABST..."
                  |   |   |   |   |
     page number:   1   2   3   4  ...

The index of guide words may be randomly searched since we know each
guide word is 4 characters long and we know where the index database
starts and ends.

Guide Word Index Searching algorythm
The following is a brief description of the searching algorythm used by
EDX in looking up a word in the dictionary.  We start by performing a
binary search on the index of guide words for a match to the first N
characters of the target word we ultimately wish to find.  There is however
the posibility that 2 or more pages of our lexical database may have the
same guide word since guide words are truncated to N characters.  We
therefore abandon the binary search if we happen to come across an exact
match, and switch to a linear search of the guide words up and down
until we positively identify the smallest possible group of pages in
which our target word must lie.

  After identifying the target range of pages of the lexical database,
we perform a linear search of those pages for our target word.  The
linear search is performed as follows:
     GIVEN:  'low'  - dictionary page number to begin search on,
                      below which word would not lie.
             'high' - upper bound dictionary page number above which
                      the word would not lie.
             'lexdba' - Starting virtual address of lexical database.
             'DICPLN' - length (in bytes) of a dictionary page.

     1.  Starting at lexdba+low*DICPLN, search forward character by
         character looking for a length-byte, which is identified by
         being less than ascii(32)
     2.  Check if length-byte = 0 indicateing End Of Lexical Database.
           If length-byte = 0 then Word Not Found.
     3.  Check if length-byte = length of target word
            If not then this can't be a match to the word we're looking for.
     4.     Else compare found word with target word starting with last
               character and moving to front of word.  We do this because
               we already expect the first few characters of the word to
               match if we're looking in the right area of the dictionary.
     5.        If match then WORD FOUND -&gt; EXITLOOP
     6.        Move forward length-byte + 1 bytes to next word.
               Check if length-byte address is beyond range we are searching.
               If so then WORD NOT FOUND -&gt; EXITLOOP
               Else go to step 2 and loop.


COMMON WORD LIST:
The common word list is formatted the same as the lexical database
section.  It consists of a list of commonly used words with a
length-byte preceeding each word.  A NULL character follows the last
word in the list.  This is a short list (about 1 block in length) of
very frequently used words, ordered by frequency of use, with the most
frequently used word listed first.  This list is searched first before a
search through the index and main lexical database is made.


LOADING THE EDX DICTIONARY:
The EDX dictionary is not "read" into memory.  Instead it is "mapped" into
memory using the $CRMPSC service, which is considerably faster and doesn't
use up any user page file quota (pgflquo).

The dictionary could be loaded by first allocating about 2000 pages of
memory, and then reading the entire file into the memory allocated.  This
would be quite slow due to the large size of the database.  Also a user's
pgflquo quota limits the total amount of memory a user may allocate, and
the 2000 pages required for the database is a considerable amount of memory.

$CRMPSC accomplishes the same result of allocating memory and then reading
the file into memory, except it never allocates memory from the system, and
it never reads in the file.  Instead, it expands the process region by 2000
pages thereby instantly making new virtual memory available, and then
it declares that the the physical file EDX_DICTIONARY.DAT itself is the
paging file for that section of memory.  The initialization is now complete,
with hardly any work having been done.

Now when the program attempts to read some of the dictionary that's in that
memory range, a page fault will occur if that page is not already in memory
and that page is automatically read into memory.  And since we're not using
the system paging file for this, the user's pgflquo quota is not affected.


PERFORMANCE OPTIMIZATION
There are 3 variables described above which affect the performance of
this dictionary.  You may play with these variables if you wish, and see
what values work best.  The varibles are:
     DICINDSWD  ;INDSWD.  Size of guide word in index
     DICDICPLN  ;DICPLN.  Dictionary Page Length in bytes
     PFC - Page Fault Cluster Size argument to $CRMPSC
(INDSWD and DICPLN are changed in EDX_BLDDIC.C and then the file
EDX_DICTIONARY.DAT is recreated using the new variables.  PFC is the
12th and last argument in the call to $CRMPSC in file EDX_CALLUSER.C)

INDSWD: (Index Size of Word)
The smaller the guide word size, the smaller the index, and the less
time it takes to search the index.  However, a smaller guide word means
the range of pages where a target word must lie may not be as narrow as
it would if the guide words were longer.  For example, if the guide word
length were 2, and you were looking up the word 'ASSIMULATE', you would
have to search all of the lexical database pages which had a guide word of
'AS'.

Conversely, with a larger guide word size, the range of pages wherein a
target guide word must lie can be narrowed down more.  With a very large
guide word size you could narrow down the search for a particular word
to the exact page.  Narrowing this range down means less of the lexical
database neads to be read and searched, but it also means the index gets
larger.

DICPLN:(Dictionary Page Length)
With a small page size there are more pages total, which means more
guide words, which means a larger index, but it also means it may be
possible to further narrow the range of pages wherein a target word must
lie.  This means less linear searching of pages for the target word.
However, determining the range of pages wherein a word must lie is also
dependent upon the size of the guide word used (INDSWD).

Conversely, a large page size means less guide words, a smaller index
size, and faster searching of the index.  Changing the page size from
512 to 1024 will cut the size of the index in half.  However, when the
range of pages is determined, those pages will have to be searched to
find the target word.

Recommendations:
The index guide word size should be as large as possible and the page length
should be as small as possible so that the position of most words
in the lexical database can be narrowed down as much as possible.  The
increase in size of the index is small and only affects the total size of
the file EDX_DICTIONARY.DAT, of which the index accounts for only about 1%.
It does not adversely affect the process since none of the EDX dictionary
uses up virtual page file quota (pgflquo).  (It is mapped into memory as
process private non-modifiable disk section using $CRMPSC).

As such we have set the dictionary page size 512 bytes (1 pagelet), and
the index guide word length to 6 which seems to be long enough so that
there are few instances of two guide words being the same.

PFC:(Page Fault Cluster Size argument to $CRMPSC)
This is the 12th and last argument to the system service routine
$CRMPSC.  This determines how many pagelets (512 byte pages) get swapped
in each time a page fault occurs accessing the lexical database.  A
small number here means when a page fault occurs less time is spent
reading in the cluster of pages, but more page faults overall are likely
to occur.  A large number here means when a page fault does occur we
page in a lot of pages.  Page faulting in 1 page at a time 500 times is
a lot slower than paging in 500 pages at once.  Tests also show that
small PFC size can cause the spelling checker to go twice as slow.  It
is also intuitive that if we are spell checking a document, we will make
heavy use of the entire lexical database during that time and it would
be nice to page in as much as possible up to the limit of the users
working set quota.  On AXP the real page size is either 8K, 16K, 32K, or
64K.  We have chosen the page size to be 64K * 4 = 500 pagelets.


OPTIMIZING THE COMMON WORD LIST:
The common word list is searched first for a match before searching
the main lexical database.  Common words which occur often are thus
handled quickly.  A longer common word list means a higher chance of
matching whatever the target word is, thus skipping the longer process
of searching The main lexical database.  However, a longer common word
list means more time spent searching the common word list.

Examining a reference book which lists words according to frequency of
use is helpful in determining which words should be included in a
common word list, and how long The list should be.  It turns out that
The first few words at The top of The list are used quite frequently,
with The number one word at The very top of The list being used nearly 3
times as often as The second word on The list.  (And by now you may have
guessed that The number one most frequently used word in The English
language is The word 'THE').

Probably a list containing the first 10 most commonly used words would
be as effective in speeding up a spelling checker as a word list of the
first 100 most commonly used words.

All characters in the dictionary file EDX_DICTIONARY.DAT are in lowercase.

TESTING:
To test the spell checking code try spell checking it's own dictionary.
If it can't spell check that something's wrong.  Also need to try some
('s) words.  Also try spell checking the user's personal dictionary.
---------------------------------------------------------------------------*/

/* Constants */
#define BLOCK_SIZE   512                /* Number of bytes in a block */
#define WDBUF_SIZE    80                /* Inword buffer */

#define DICVERNO      3                 /* EDX Dictionary Version Number */
static const char DICID[8] = { DICVERNO, 'E', 'D', 'X', 'd', 'i', 'c', 't' };

/* GLOBAL SPELL CHECK VARIABLES */
#define WORD_COLUMN_LENGTH 20

static int gmode;               /* guess mode    (1=reversals,2=vowels,3=minus,4=plus,5=consonants,6=giveup) */
static int gsubmode;            /* guess submode (letter we're currently replacing with) */
static int gof;                 /* guess column offset (character # in word working on) */

static char *newnode = 0;               /* pointer to structure node, the new node added to the tree */
static int32 accept_tree = 0;           /* header for accepted word tree */
static int32 savcor_tree = 0;           /* header for saved corrections tree */

static char *dicptra;           /* points to length-byte preceeding first word displayed on screen by dic_browse */
static char *dicptrz;           /* points to length-byte following last word displayed on screen by dic_browse */
static char target_word[80];    /* word spelling checker is currently checking */
static int target_word_len;     /* length of above */
static char dic_lwa[80];        /* copy of last misspelled word */
static int dic_lwl;             /* last misspelled word length */

/* SPELL CHECKER GLOBAL VARIABLES */
static struct FAB dicfabio;
static struct RAB dicrabio;
static struct NAM dicnamio;
static struct FAB dicfabmap;
static struct FAB persdicfab;
static struct RAB persdicrab;
static char *cmnwdsptr = 0;     /* Address of common words */
static char *dicindptr = 0;
static int  dic_loaded = FALSE; /* TRUE when EDX dictionary successfully loaded */

static char  wdbuf[WDBUF_SIZE];

static int32 maprange[2] = { 0x200, 0x200 };    /* any program (P0) region address */

static struct
{
   char *lexdba;        /* Lexical Database Address (address range returned here as 2 longwords begin:end) */
   char *lexend;        /* ASSUME sizeof(lex) = longword */
} dic;

static struct dichead_layout {
   char id[8];                          /* header id */
   int32 lexvbn;  int32 lexvbn_h;       /* Lexical Database Virtual Block Number (VBN it starts at) */
   int32 lexbln;  int32 lexbln_h;       /* Lexical Database Block Length (length in 512 byte blocks) */
   int32 indvbn;  int32 indvbn_h;       /* Index Length (in bytes) */
   int32 indlen;  int32 indlen_h;       /* Index Length (in bytes) */
   int32 indswd;  int32 indswd_h;       /* Index Size of guide Word (in bytes) */
   int32 dicpln;  int32 dicpln_h;       /* Dictionary Page Length (in bytes) */
   int32 cwdvbn;  int32 cwdvbn_h;       /* Commonwords Virtual Block Number (VBN it starts at) */
   int32 cwdlen;  int32 cwdlen_h;       /* Commonwords Length (in bytes) */
   int32 cwdmln;  int32 cwdmln_h;       /* Commonwords Maximum Length (in bytes) */
} dichead;

/*---------------------------------------------------------------------------*/
void edx_spell()
{
   int status;

   if (!dic_loaded)                     /* if not spell dic already mapped */
   {                                    /* then map it into memory */
      status = spell_init();            /* Initialize spelling checker */
      if ( !(status & STS$M_SUCCESS) )  /* Error initializing dictionary */
      {
         fmtoutstr( 1, &status );
         return;
      }
   }

   switch (*incode_ptr)
   {
      case INCODE_SPELL_DIC_BROWSE_PA:                                   /* 1 = Dictionary browse previous page */
      case INCODE_SPELL_DIC_BROWSE_RW:                                   /* 2 = Dictionary browse using word */
      case INCODE_SPELL_DIC_BROWSE_PZ:     dic_browse();         break;  /* 3 = Dictionary browse next page */
      case INCODE_SPELL_TEXTLINE:          spell_textline(&instr_desc);
                                                                 break;  /* 4 = Spell textline */
      case INCODE_SPELL_GUESS:             spell_guess();        break;  /* 5 = Spell guess */
      case INCODE_SPELL_ACCEPT_WORD:       spell_accept_word();  break;  /* 6 = Accept word (add to accepted word list) */
      case INCODE_SPELL_ADD_PERSDIC:       add_persdic();        break;  /* 7 = Add word to personal dictionary */
      case INCODE_SPELL_DUMP_COMMONWORDS:  dump_commonwords();   break;  /* 8 = Dump commonword list */
      case INCODE_SPELL_SAVE_CORRECTION:   save_correction(&instr_desc);
                                                                 break;  /* 9 = Save misspelled word and its correction */
      default:  edx_signal(1, byref(EDX__UNKNCODE) ); break;                     /*Unknown item code */
   }
}

/*---------------------------------------------------------------------------

        .SUBTITLE SPELL_INIT

 Functional Description:
        Initializes the EDX dictionary.  Opens all necessary files,
        reads in all necessary data.  On error prints error and returns
        error status.
        Uses system service routine $CRMPSC to map the dictionary straight
        into memory.  This method does not use up any user pgflquo quota.

 Calling Sequence:
        status = spell_init();

 Outputs:
        status

 Outline:
        1.  FABs, RABs, NAMs, etc initialized.
        2.  The EDX dictionary database file EDX_DICTIONARY.DAT is opened
            and connected to.
        3.  The rest of the file is mapped into memory using $CRMPSC.
        4.  Pointers to the index and common words are set.  (Pointer
            to the lexical database itself was set by call to $CRMPSC)
        5.  User's personal dictionary file, if one is found, is opened
            and the words there are inserted into the binary tree of
            accepted words.

---------------------------------------------------------------------------*/

int spell_init()
{
   int status;
   int sig_array[6];
   char filename[NAM$C_MAXRSS];
   struct dsc$descriptor_s wdbuf_desc = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, wdbuf };

   edx_signal(1, byref(EDX__DICLOAD) );                 /* Say "Loading dictionary" */

   /* Initialize DICNAMIO */
   dicnamio = cc$rms_nam;                               /* initialize NAM */
   dicnamio.nam$l_esa = (char *) filename;              /* Expanded file name string */
   dicnamio.nam$b_ess = NAM$C_MAXRSS;                   /* Expanded file name string size */

   /* Initialize DICFABIO */
   dicfabio = cc$rms_fab;                               /* initialize FAB */
   dicfabio.fab$l_fna = (char *) "EDX_DICTIONARY";      /* file name address */
   dicfabio.fab$b_fns = LENGTH("EDX_DICTIONARY");       /* file name length  */
   dicfabio.fab$l_dna = (char *) "SYS$LIBRARY:EDX_DICTIONARY.DAT";      /* default file name address */
   dicfabio.fab$b_dns = LENGTH("SYS$LIBRARY:EDX_DICTIONARY.DAT");       /* default file name length  */
   dicfabio.fab$l_nam = &dicnamio;                      /* NAM block address */
   dicfabio.fab$b_fac = FAB$M_BIO | FAB$M_GET;          /* File Access options = Block I/O, GET */
   dicfabio.fab$b_shr = FAB$M_SHRGET;                   /* share options */

   /* Initialize DICRABIO */
   dicrabio = cc$rms_rab;                               /* initialize RAB */
   dicrabio.rab$l_fab = &dicfabio;                      /* address of FAB */

   /* Initialize DICFABMAP */
   dicfabmap = cc$rms_fab;                              /* initialize FAB */
   dicfabmap.fab$l_fna = (char *) "EDX_DICTIONARY";     /* file name address */
   dicfabmap.fab$b_fns = LENGTH("EDX_DICTIONARY");      /* file name length  */
   dicfabmap.fab$l_dna = (char *) "SYS$LIBRARY:EDX_DICTIONARY.DAT";     /* default file name address */
   dicfabmap.fab$b_dns = LENGTH("SYS$LIBRARY:EDX_DICTIONARY.DAT");      /* default file name length  */
   dicfabmap.fab$l_fop = FAB$M_UFO;                     /* User File Open */
   dicfabmap.fab$b_fac = FAB$M_GET;                     /* UPI must be set says the book */
   dicfabmap.fab$b_shr = FAB$M_SHRGET | FAB$M_UPI;      /* share options */
   dicfabmap.fab$b_rtv = -1;                            /* keep all pointers */

   /* Initialize PERSDICFAB */
   persdicfab = cc$rms_fab;                             /* initialize FAB */
   persdicfab.fab$l_fna = (char *) "EDXPERSDIC";        /* file name address */
   persdicfab.fab$b_fns = LENGTH("EDXPERSDIC");         /* file name length  */
   persdicfab.fab$l_dna = (char *) "SYS$LOGIN:EDXPERSDIC.DAT";  /* default file name address */
   persdicfab.fab$b_dns = LENGTH("SYS$LOGIN:EDXPERSDIC.DAT");   /* default file name length  */
   persdicfab.fab$b_fac = FAB$M_GET;                    /* File Access options = Block I/O, GET */
   persdicfab.fab$b_shr = FAB$M_SHRGET;                 /* share options */

   /* Initialize PERSDICRAB */
   persdicrab = cc$rms_rab;                             /* initialize RAB */
   persdicrab.rab$l_fab = &persdicfab;                  /* address of FAB */
   persdicrab.rab$l_ubf = (char *) wdbuf;               /* address of buffer */
   persdicrab.rab$w_usz = WDBUF_SIZE;                   /* buffer size */


   /* OPEN AND CONNECT TO EDX_DICTIONARY */
   status = sys$open(&dicfabio);
   if (status & STS$M_SUCCESS)
      status = sys$connect(&dicrabio);
   if ( !(status & STS$M_SUCCESS) )
   {
      lib$put_output(&bell_desc);                       /* Ring terminal bell */
      sig_array[0] = EDX__ERROPENDIC;                   /* error opening dictionary file message */
      sig_array[1] = 2;                                 /* 2 FAO args */
      sig_array[2] = dicnamio.nam$b_esl;                /* filename size */
      sig_array[3] = dicnamio.nam$l_esa;                /* filename address */
      sig_array[4] = status;                            /* Error status */
      sig_array[5] = 0;                                 /* 0 FAO args */
      edx_signal( 6, sig_array );
      return(EDX__ERROPENDIC);
   }

   /* READ IN DICTIONARY HEADER */
   dicrabio.rab$l_bkt = 1;                      /* Block number to read */
   dicrabio.rab$l_ubf = &dichead;               /* Buffer to put it all in */
   dicrabio.rab$w_usz = sizeof(dichead);        /* Buffer size = 1 BLOCK */
   status = sys$read(&dicrabio);                /* read in dictionary header block */
   if ( !(status & STS$M_SUCCESS) )
   {
      sig_array[0] = EDX__SPLINITERR;                   /* spell initialization error */
      sig_array[1] = 0;                                 /* 0 FAO args */
      sig_array[2] = status;                            /* error status */
      sig_array[3] = 0;                                 /* o FAO args */
      edx_signal( 4, sig_array );
      return(EDX__SPLINITERR);
   }

   /* CHECK VALIDITY OF HEADER */
   if (memcmp(&dichead.id,DICID,8))             /* = 0 if they match, which = FALSE */
   {
      if (!memcmp(&dichead.id+1,&DICID[1],3))   /* version #2 only had ".EDX" */
      {
         sig_array[0] = EDX__DICVERSERR;        /* dictionary version error. */
         sig_array[1] = 2;                      /* 2 FAO arguments */
         sig_array[2] = (int) dichead.id[0];    /* actual version number */
         sig_array[3] = DICVERNO;               /* required version number */
         edx_signal( 4, sig_array );
         return(EDX__DICVERSERR);
      }
      else
      {
         sig_array[0] = EDX__DICHEADERR;        /* dictionary header error.  It's not the EDX_DICTIONARY file */
         sig_array[1] = 2;                      /* 2 FAO args */
         sig_array[2] = dicfabio.fab$b_fns;     /* filename size */
         sig_array[3] = dicfabio.fab$l_fna;     /* filename address */
         edx_signal( 4, sig_array );
         return(EDX__DICHEADERR);
      }
   }

   /* MAP LEXICAL DATABASE INTO MEMORY */
   sys$close(&dicfabio);                        /* close it for I/O */
   status = sys$open(&dicfabmap);               /* open it for mapping */
   if (status & STS$M_SUCCESS)
      status = sys$crmpsc( &maprange,                           /* inadr */
                           &dic,                                /* retadr (array by reference) */
                           0,                                   /* acmode */
                           SEC$M_EXPREG,                        /* flags */
                           0,                                   /* gsdnam */
                           0,                                   /* ident */
                           0,                                   /* relpag */
                           dicfabmap.fab$l_stv,                 /* chan */
                           0,                                   /* pagcnt */
                           dichead.lexvbn,                      /* vbn */
                           0,                                   /* prot */
                           500 );                               /* pfc */
   if ( !(status & STS$M_SUCCESS) )
   {
      lib$put_output(&bell_desc);                       /* Ring terminal bell */
      sig_array[0] = EDX__ERRMAPDIC;                    /* error mapping dictionary file message */
      sig_array[1] = 0;                                 /* 0 FAO args */
      sig_array[2] = status;                            /* error code */
      sig_array[3] = 0;                                 /* 0 FAO args */
      edx_signal( 4, sig_array );
      return(EDX__ERRMAPDIC);
   }

/* SET POINTERS TO INDEX AND COMMON WORDS */
   cmnwdsptr = dic.lexdba + (dichead.cwdvbn - dichead.lexvbn)*BLOCK_SIZE;
   dicindptr = dic.lexdba + (dichead.indvbn - dichead.lexvbn)*BLOCK_SIZE;
   dic_loaded = TRUE;                   /* successfully loaded enough so
                                           we can spell check */

/* OPEN AND READ IN THE USER'S PERSONAL DICTIONARY FILE */
   status = sys$open(&persdicfab);
   if (status & STS$M_SUCCESS)
      status = sys$connect(&persdicrab);

   if (status != RMS$_FNF)
   {
      if (status & STS$M_SUCCESS)
      {
         /* GO THROUGH THE USERS PERSONAL DICTIONARY, ADD THE WORDS TO OUR TREE */
         while ( sys$get(&persdicrab) & STS$M_SUCCESS )
         {
            wdbuf_desc.dsc$w_length = persdicrab.rab$w_rsz;
            spell_textline(&wdbuf_desc);        /* this trims, lowercases, and sets word for inclusion in accepted word tree */
            spell_accept_word();                /* add word to accepted word list */
         }
      }
      else
      {
         sig_array[0] = EDX__PERSDICERR;                /* error opening personal dictionary */
         sig_array[1] = 2;                              /* 2 FAO args */
         sig_array[2] = persdicfab.fab$b_fns;           /* filename size */
         sig_array[3] = persdicfab.fab$l_fna;           /* filename address */
         sig_array[4] = status;                         /* Error status */
         sig_array[5] = 0;                              /* 0 FAO args */
         edx_signal( 6, sig_array );
         /* if there's an unexpected error accessing the user's personal
            dictionary then signal the error and continue.  As long as
            we got the EDX_DICTIONARY.DAT file we can spell check */
      }
   }
   sys$close(&persdicfab);                      /* close user's personal dictionary */
   edx_signal(1, byref(EDX__DICLOADED) );       /* Say "Dictionary Loaded" */
   return(SUCCESS);
}

/*---------------------------------------------------------------------------

        .SUBTITLE SPELL_TEXTLINE

 Functional Description:
        Checks the spelling of each word in the input string

 Calling Sequence:
        spell_textline(&inbuf_desc)

 Argument inputs:
        inbuf_desc - address of descriptor of string containing words to check
                     (usually INSTR)

 Outputs:
        OUTSTR = characters 1-9 is return status value
                   LIB$_NORMAL if all words in line spelled correctly
                   LIB$_NOTFOU if a word in line was spelled incorrectly
                 characters 10-14 is decimal value of offset from start of
                   instr where misspelled word begins
                 character 15 is space character
                 characters 16-20 is decimal value of length of misspelled
                   word.

 Outline:
        2.  The next word in inbuf_desc string is parsed off
            a.  inbuf is searched for the start of a word.  The start of a
                word is any character {A...Z,a...z}.
            b.  inbuf is searched for the end of the word.  The end of the word
                is any character other than {A...Z,a...z} and the appostrophie
                ("'") character.  A special check is made to handle the
                appostrophie.  Words line "we're" or "you'd" are accepted
                as is.  Words which have a trailing "'s" such as "Mark's" or
                "Saturday's" have the "'s" trimmed off.  We ignore quotes
                which occur at the end of a word with no letters following.

                When a single quote character "'" is encountered:
                 i.   If we are at the end of the line, the quote is rejected.
                 ii.  If the next character following the quote is not a
                      letter (A-Z,a-z), the quote is rejected.
                 iii. If the next character following the quote is a "S"
                      or a "s", the word is accepted without the trailing "'s"
                      the pointer is advanced to the next character following
                      the "'s".

        3.  A dictionary lookup search is made for a match to the resulting
            parsed off word.

        4.  Loop back to step 2 until all words in INSTR have been checked.
---------------------------------------------------------------------------*/

void spell_textline( struct dsc$descriptor_s *inbuf_desc)
{
   int i;
   int wdlen;                   /* word length */
   int status;
   int delta;                   /* number of chars to skip before looking for next word */
   int sig_array[3];
   char *inend;                 /* points to one char after end of inbuf */
   char *wdptr;                 /* word pointer */
   char *wdbeg;                 /* word begin */
   char *wdend;                 /* word end */
   char corout[BUFLEN+8];
   $DESCRIPTOR(corout_desc,corout);
   char faobuf[BUFLEN];
   $DESCRIPTOR(faobuf_desc,faobuf);
   char retbuf[BUFLEN];
   $DESCRIPTOR(retbuf_desc,retbuf);
   struct dsc$descriptor_s word_desc;
   $CONST_DESCRIP(faosplout_desc,"!5UL !5UL");  /* THIS HAS CHANGED FROM 3 */
   $CONST_DESCRIP(faocorout_desc,"!5UL !5UL !AC");      /* THIS HAS CHANGED FROM 3 */

   wdptr = inbuf_desc-&gt;dsc$a_pointer;
   inend = inbuf_desc-&gt;dsc$a_pointer + inbuf_desc-&gt;dsc$w_length;
   delta = 0;
   do
   {
      /* SEARCH FOR START OF WORD */
      /* Find first alphabetic character in set {A...Z,a...z} or end-of-line */
      for ( wdptr += delta, delta = 0;
           (wdptr &lt; inend) && !isalpha(*wdptr); /* check for out of bounds before checking character */
            ++wdptr );
      if (wdptr == inend)                       /* end of line found.  No more words on this line */
      {
         fmtoutstr(1, &LIB$_NORMAL );
         return;
      }
      wdbeg = wdptr;                    /* save start of word */

      /* SEARCH FOR END OF WORD */
      for (;;)
      {
         /* span over alphabetic {A...Z,a...z} until non-alphabetic or end-of-line found. */
         for ( ; (wdptr &lt; inend) && isalpha(*wdptr); ++wdptr ); /* Check for out of bounds before checking character */
         if (wdptr == inend) break;             /* if end-of-line then accept word */
         if (*wdptr != '\'') break;             /* if non-alphabetic and not single quote (') then accept word.  (MOST COMMON LOOP EXIT with *wdptr == SPACE ) */
         if (++wdptr == inend){--wdptr; break;} /* If no next character (end-of-line) then accept word without (') */
         if (!isalpha(*wdptr)){--wdptr; break;} /* if apostrophe not followed by alphabetic characer then accept word without (') */
         if (*wdptr == 's' || *wdptr == 'S'){--wdptr; delta = 2; break;}        /* accept word without ('S)  delta=2 -&gt; skip over 'S before searching for next word on line */
         /* else we have apostrophe followed by alphabetic non-S char.
            As in "you're", "we'll", etc.  These are checked as whole words.
            Continue loop searching for end-of-word */
      }  /* END SEARCH FOR end-of-word LOOP */

      /* On above loop exit:
           wdptr points to character after end of word
           wdbeg points to beginning of word
           length of word is wdptr - wdbeg
           delta = # of chars to skip past word end before
            searching for beginning of next word on line. */

      wdend = wdptr;
      wdlen = wdend - wdbeg;
      status = dic_lookup_word( wdlen, wdbeg );         /* see if word is in dictionary */
   }
   while ( (status & STS$M_SUCCESS) );


   /* FOUND MISSPELLED WORD */
   /* Move misspelled word to storage place (DIC_LWA)
      and Lowercase the string */
   for ( wdptr = wdbeg, i = 0;
         wdptr &lt; wdend && i &lt; BUFLEN;           /* save only as much as we can */
         ++wdptr, ++i )
      dic_lwa[i] = _tolower(*wdptr);
      dic_lwl = i;

   gmode = 1;                                   /* reset guess mode pointers */
   gsubmode = gof = 0;

   /* set up response string to EDX */
   /* check misspelled word list for possible suggested correction */
   word_desc.dsc$w_length = dic_lwl;
   word_desc.dsc$b_class = DSC$K_CLASS_S;
   word_desc.dsc$b_dtype = DSC$K_DTYPE_T;
   word_desc.dsc$a_pointer = dic_lwa;
   if ( lib$lookup_tree(&savcor_tree,&word_desc,compare_node,&newnode) & STS$M_SUCCESS )
   {
      /* correction word found, make response and include it. */
      sys$fao(&faocorout_desc,                  /* ctrstr without suggested correction (by descriptor) */
              &corout_desc.dsc$w_length,        /* (write) outlen */
              &corout_desc,                     /* outbuf (by descriptor) */
              wdbeg - inbuf_desc-&gt;dsc$a_pointer,/* offset into textline where misspelled word starts */
              wdlen,                            /* length of misspelled word */
              newnode + 12 + *(int16 *)(newnode+10) );  /* address of ASCIC string which is suggested correction */
   }
   else
   {
      /* correction word not found, make response without it */
      sys$fao(&faosplout_desc,                  /* ctrstr without suggested correction (by descriptor) */
              &corout_desc.dsc$w_length,        /* (write) outlen */
              &corout_desc,                     /* outbuf (by descriptor) */
              wdbeg - inbuf_desc-&gt;dsc$a_pointer,/* offset into textline where misspelled word starts */
              wdlen);                           /* length of misspelled word */
   }

   /* final output string */
   sig_array[0] = status;
   sig_array[1] = corout_desc.dsc$w_length;
   sig_array[2] = corout;
   fmtoutstr( 3, &sig_array );
}

/*=============================================================================

        .SUBTITLE DIC_LOOKUP_WORD

 Functional Description:
        Searches the EDX dictionary for a given word

 Calling Sequence:
        status = dic_lookup_word(wdlen, wdbeg);

 Argument inputs:
        wdlen - length of word to search for (by value)
        wdbeg - pointer to start of word array

 Outputs:
        status = LIB$_NORMAL - word was found
               = LIB$_NOTFOU - word was not found

 Outline:
        1.  The input word is copied to target_word buffer and lowercased.

        2.  The dictionary common word list is searched for the word.

        3.  The main lexical database is searched for the word.

        4.  The list of accepted words is searched for a match.

---------------------------------------------------------------------------*/

int dic_lookup_word(int wdlen, char *wdbeg)
{
   int i;
   int low;             /* lower bound page # */
   int high;            /* upper bound page # */
   char *wdend;         /* word pointer */
   char *wdptr;         /* word pointer */
   char *dptr;          /* pointer into dictionary into word */
   char *lbptr;         /* pointer to length-byte of current word */
   char *tptr;          /* pointer into target_word */
   char *endrange;
   struct dsc$descriptor_s target_word_desc;
   /* NOTE: pages referred to are edx_dictionary pages of size DICPLN */

   /* SETUP_DICWORD */
   /* Move misspelled word to storage place, lowercase, and blank pad
      to INDSWD in length (so we can compare it with guide words) */
   if (wdlen == 0) return(LIB$_NORMAL);         /* accept zero length word as OK */
   if (wdlen &gt; BUFLEN) return(LIB$_NOTFOU);     /* Word over 255 chars long.  Can't possibly be a word.  User probably doesn't want us to stop on it anyway. */
   wdend = wdbeg + wdlen;                       /* wdend -&gt; char after last char of word */
   for ( wdptr = wdbeg, i = 0;
         wdptr &lt; wdend;
         ++wdptr, ++i )
      target_word[i] = _tolower(*wdptr);
   target_word_len = wdlen;

   for ( ; i &lt; dichead.indswd; ++i )            /* BLANK PAD TO INDSWD LENGTH */
      target_word[i] = SPACE;

/* SEARCH COMMON WORD LIST FOR MATCH */
   if (target_word_len &lt;= dichead.cwdmln)                       /* skip if target_word is too long to be in commonword list */
   {
      endrange = cmnwdsptr + dichead.cwdlen;                    /* end of commonwords */
      lbptr = cmnwdsptr;                                        /* start at beginning of common words */
      while (lbptr &lt; endrange)                                  /* still in range of dictionary we're searching */
      {
         if (*lbptr == 0x00) break;                             /* End of Lexical Database */
         if (*lbptr == target_word_len)                         /* check if word lengths match first */
         {
            for ( tptr = target_word + target_word_len -1,      /* start tptr at last char of target_word */
                  dptr = lbptr + target_word_len;               /* start dptr at last char of word in dictionary */
                  tptr &gt;= target_word && *tptr == *dptr;        /* while chars match up to beginning of word */
                  --tptr, --dptr);                              /* move back a char */
            if (tptr &lt; target_word) return(LIB$_NORMAL);        /* word found */
         }
         lbptr += *lbptr + 1;                                   /* move to next word */
      }
   }

/* SEARCH MAIN DICTIONARY FOR MATCH */
   binsrch_maindic( &low, &high, &target_word );

/* Linear search dictionary pages for match to target word.  Compare
   found word with target word starting with last character and moving
   to front of word.  We do this because we already expect the first
   few characters of the word to match if we're looking in the right
   area of the dictionary.
*/
   endrange = dic.lexdba + high * dichead.dicpln;
   for ( lbptr = dic.lexdba + low * dichead.dicpln; *lbptr &gt; 31; ++lbptr);      /* find a length-byte */
   while (lbptr &lt; endrange)
   {
      if (*lbptr == 0x00) break;                                /* End of Lexical Database */
      if (*lbptr == target_word_len)                            /* check if word lengths match first */
      {
         for ( tptr = target_word + target_word_len -1,         /* start tptr at last char of target_word */
               dptr = lbptr + target_word_len;                  /* start dptr at last char of word in dictionary */
               tptr &gt;= target_word && *tptr == *dptr;           /* while chars match up to beginning of word */
               --tptr, --dptr);                                 /* move back a char */
         if (tptr &lt; target_word) return(LIB$_NORMAL);           /* word found */
      }
      lbptr += *lbptr + 1;                                      /* move to next word */
   }

/* DROP OUT BOTTOM IF WORD NOT FOUND IN MAIN DICTIONARY */
/* SEARCH ACCEPTED WORD LIST FOR MATCH */
   target_word_desc.dsc$w_length = target_word_len;
   target_word_desc.dsc$b_class = DSC$K_CLASS_S;
   target_word_desc.dsc$b_dtype = DSC$K_DTYPE_T;
   target_word_desc.dsc$a_pointer = target_word;
   if ( lib$lookup_tree(&accept_tree,&target_word_desc,compare_node,&newnode) & STS$M_SUCCESS )
      return(LIB$_NORMAL);              /* WORD FOUND IN ACCEPTED WORD LIST */
   else
      return(LIB$_NOTFOU);              /* WORD NOT FOUND ANYWHERE.  SORRY */
}
/*---------------------------------------------------------------------------

        .SUBTITLE BINSRCH_MAINDIC

 Functional Description:
        The index to the dictionary main lexical database is searched
        to determine the page range within which target_word must lie
        if it exists.

 Calling Sequence:
        binsrch_maindic( &low, &high, &target_word );

 Argument inputs:
        target_word - character array of word to match, blank padded to
                      dichead.indswd (by reference)

 Outputs:
        low - page number below which word would not reside (by reference)
        high - high page number above which word would not reside (by reference)

Notes: The first page number is 0.
---------------------------------------------------------------------------*/
void binsrch_maindic( int *low,
                      int *high,
                      char *target_word )
{
   int cmp;
   int new;

/* PREPARE FOR BINARY SEARCH */
   *high = dichead.indlen/dichead.indswd - 1;
   *low = 0;

/* BINARY SEARCH THE INDEX */
   for (;;)
   {
      new = (*low + *high)/2;
      if (new == *low) break;                   /* exitloop when guess=lowb */
      cmp = memcmp(target_word, dicindptr + new*dichead.indswd, dichead.indswd);
      if (cmp == 0) break;                      /* switch to linear search */
      if (cmp &gt; 0) *low = new;                  /* word is in higher half */
      else *high = new;                         /* else word is in lower half */
   }/*endloop */

/* NOW DO LINEAR SEARCH UP AND DOWN TO FIND TRUE PAGE BOUNDARIES */
/* FIRST LOOK TOWARD Z'S FOR NEW &gt; TARGET_INDEX OR END OF DICTIONARY */
/* Set upper bound page #.  NOTE: Search is to INCLUDE this page up to
first length-byte.  Word we are looking for may be at the end of prev
page spilling over into this page.  ALSO NOTE: if word is in very last
page of dictionary past last guide word then we must increment high by
one to include the last page.  */
   *high = dichead.indlen/dichead.indswd - 1;   /* number of last page in dictionary */
   while( (cmp = memcmp(target_word,
                       dicindptr + new*dichead.indswd,
                       dichead.indswd))
          &gt;= 0  &&  new != *high ) ++new;
   *high = new;                         /* Set upper bound page # */
   if (cmp &gt;= 0) ++*high;               /* include very last page of dictionary if need be */


/* SEARCH FOR NEW &lt; TARGET_INDEX OR BEGINNING OF DICTIONARY */
   while( memcmp(target_word,
                 dicindptr + new*dichead.indswd,
                 dichead.indswd)
          &lt;= 0  &&  new != 0 ) --new;
   *low = new;  /* set lower bound page # */
}

/*=============================================================================

        .SUBTITLE DIC_BROWSE

 Functional Description:
        Returns words from the dictionary for EDX to display in it's
        dictionary lookup buffer.  This routine returns in OUSTSTR a
        very long string of length ROWS x COLUMNS which is to be broken
        up by EDX into pieces of length COLUMNS and displayed.

        We either do a lookup best match to given word, a display next
        page, or a display previous page, depending upon the value of R6.

 Calling Sequence:
        dic_browse();

 Argument inputs:
                 ....v....1....v....2....v....3....v....4....v....5....v....6...
        INSTR - "yyyyyyyyzzzzzzzzwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww...
                where yyyyyyyy is ascii for a hex longword indicating
                    the number of rows in the DIC window to fill
                and zzzzzzzz is ascii for a hex longword indicating
                    the number of columns in the DIC window to fill
                    and wwwwwwwwwwww... is the word to best match if
                    we are not doing a prev_page or next_page display.

 Implicit inputs:
        incodel - 1 display previous page
                  2 do best match word and display page
                  3 display next page

 Outputs:
        OUTSTR - a string of length ROWS x COLUMNS (yyyyyyyy x zzzzzzzz)
                containing all the best match words in alphabetical order
                and arranged so that the first n characters is to be put
                as row 1, the next n characters as row 2, etc...

        For typical values of 10 ROWS and 80 COLUMNS the output is displayed
        as 4 columns, each word a maximum length of 19 characters else
        the word overflows into the next column.

            Example final output display:

                word1           word11          word21          word31
                word2           word12          word22          word32
                word3           word13          word23          word33
                word4           word14          word24          word34
                word5           word15          word25          word35
                word6           word16          word26          word36
                word7           word17          word27          word37
                word8           word18          word28          word38
                word9           word19          word29          word39
                word10          word20          word30          word40

        The above typical example has 4 word columns and 10 rows.
        In the above example we'd try to make our best fit word word21.

 Implicit:
        Pointers DICPTRA and DICPTRZ are set.  DICPTRZ points to the
        length-byte following the last word in the lexical database
        displayed on the screen.  DICPTRA points to the length-byte
        preceeding the first word in the lexical database displayed.
        These pointers are used if the user requests to see the next
        or previous screen full of words in the dictionary.

 Outline:
        1.  INSTR is parsed, yyyyyyyy, zzzzzzzz, are converted to integers.
        2.  Depending on incodel we either call dic_browse_prev_page,
            dic_browse_word, or dic_browse_next_page (which is just calling
            dic_browse_fill passing it DICPTRZ)

---------------------------------------------------------------------------*/

void dic_browse()
{
   unsigned int status;
   unsigned int incodel;
   unsigned int rows;
   unsigned int columns;
   struct dsc$descriptor_s numrows_desc = { 8, DSC$K_DTYPE_T, DSC$K_CLASS_S, instr_desc.dsc$a_pointer };
   struct dsc$descriptor_s numcols_desc = { 8, DSC$K_DTYPE_T, DSC$K_CLASS_S, instr_desc.dsc$a_pointer+8 };

   incodel = (*incode_ptr & 0x0000FFFF);

   status = ots$cvt_tz_l( &numrows_desc, &rows );                       /* convert ascii hex to binary */
   if ( !(status & STS$M_SUCCESS) ) lib$signal(status);                 /* signal error */

   status = ots$cvt_tz_l( &numcols_desc, &columns );                    /* convert ascii hex to binary */
   if ( !(status & STS$M_SUCCESS) ) lib$signal(status);                 /* signal error */

   switch (incodel)
   {
      case 1:                                                   /* 1 = display prev page */
              dic_browse_prev_page( columns, rows );
              break;
      case 2:                                                   /* 2 = lookup word.  Use wwwwwwwwwww... word in INSTR */
              dic_browse_word( instr_desc.dsc$w_length-16,      /* length - 16 to skip over yyyyyyyyzzzzzzzz */
                               instr_desc.dsc$a_pointer+16,     /* address of instr+16 (wwwwwwwwwwwwwww... word) */
                               columns, rows );
              break;
      case 3:                                                   /* 3 = display next page */
              dic_browse_fill( dicptrz,                         /* fill starting with this word */
                               columns, rows);
              break;
      default:                                                  /* Unknown item code */
              edx_signal(1, byref(EDX__UNKNCODE) );
              break;
   }
}

/*---------------------------------------------------------------------------

        .SUBTITLE DIC_BROWSE_PREV_PAGE

 Functional Description:
        Starting at DICPTRA, counts backwards an appropriate number of
        words, and calls dic_browse_fill to do the rest given the starting
        point.

 Calling Sequence:
        dic_browse_prev_page( window_columns, window_rows );

 Argument inputs:
        window_columns - number of characters across window display (width)
        window_rows  - number of rows in window display to fill (height)

 Outline:
        1.  Count backwards N words (N = window_rows).  In the above example
            these words will occupy word31 - word40, the right most column.

        2.  Count backwards M more words.  In the above example these words
            will occupy word1 - word30.  In general M is the number of words
            necessary to fill all the previous columns we have.  M is
            calculated as follows:
                M = ( total_number_of_word_columns - 1 ) * window_rows  ( if M &lt; 0 then M = 0 )

        where total_number_of_word_columns = INT( window_columns / WORD_COLUMN_LENGTH )

            If a word is longer than WORD_COLUMN_LENGTH-1, then we count it
            as two words.  For example, in the above example if word3 were
            longer than WORD_COLUMN_LENGTH-1, it would spill into the column
            for word13.  Thus this word effectively takes up two word spaces.
---------------------------------------------------------------------------*/

void dic_browse_prev_page( int window_columns, int window_rows)
{
   int n,m;
   int word_length;
   char *dptr;

/* Step 1.  Count N words backwards not looking at word length */
/* GO BACK N WORDS */
   dptr = dicptra;
   for ( n = window_rows; n &gt; 0; --n )
   {
      if (dptr == dic.lexdba) break;            /* don't back up any more if we're at the beginning of the database */
      for( --dptr;                              /* backup one char to get off length-byte */
           *dptr &gt; 31;                          /*  then while not on length-byte */
           --dptr );                            /*   move back a char */
   }


/* Step 2.
     Count M words backwards looking at word length
     if word length &gt;= word_column_length-1 then count as two words
     total_number_of_word_columns = INT( window_columns / WORD_COLUMN_LENGTH )
     M = ( total_number_of_word_columns - 1 ) * window_rows
*/
   for ( m = ( (window_columns/WORD_COLUMN_LENGTH) - 1 ) * window_rows;
         m &gt; 0                  /* Count M words backwards */
      && dptr &gt; dic.lexdba;     /* check for at beginning of dictionary */
         --m)
      /* GO BACK ONE WORD (or WORD_COLUMN_LENGTH-1 chars, whichever comes first) */
      for ( word_length = 0, --dptr;            /* Start with zero word_length */
            *dptr &gt; 31                          /* While not found length-byte (separating words) */
         && word_length != WORD_COLUMN_LENGTH-1;/*  and word still fits in column */
            ++word_length, --dptr );            /* Increment word_length, backup a char */

   /* IN CASE WE ENDED UP IN THE MIDDLE OF A LONG WORD, MOVE FORWARD TO
      THE FIRST LENGTH-BYTE.  (USUALLY WE'RE ALREADY ON IT AT THIS POINT) */
   for ( ; *dptr &gt; 31; ++dptr);         /* move forward to first length-byte (probably already on it) */

/* NOW CALL DIC_BROWSE_FILL TO FILL OUTSTR WITH
   WORDS FROM DICTIONARY STARTING AT dptr */
   dic_browse_fill( dptr, window_columns, window_rows );
}

/*-------------------------------------------------------------------------

        .SUBTITLE DIC_BROWSE_WORD

 Functional Description:
        Accepts a word defined by inwdlen, inwdbeg.  Searches in the dictionary
        for the best match to the given word.  Counts backwards an
        appropriate number of words, and calls dic_browse_fill to
        do the rest given the starting point.

 Calling Sequence:
        dic_browse_word( inwdlen, inwdbeg, window_columns, window_rows );

 Argument inputs:
        number of arguments = 4
        inwdlen = length of string containing word to best match
        inwdbeg = pointer to string containing word to best match
        window_columns = number of characters across window display (width)
        window_rows = number of rows in window display to fill (height)

 Outline:
        1.  The given word is copied over to local storage, lowercased,
            and blank padded to INDSWD in length.

        2.  The index to the dictionary main lexical database is searched
            to determine the page range within which the word must lie if
            it exists.

        3.  Search range of dictionary pages for match to word.  If no match
            found search again for word(1:length-1).  If no match found search
            for word(1:length-2), and so on until we reach search for word(1:0),
            which guarantees a match.

        4.  Find best match word.  Search forwards until
            current_word &gt; target_word.

        5.  Count backwards N words.  In the example given in DIC_BROWSE,
            these words will occupy word11 - word20, the second column.
            N = window_rows.

        6.  Count backwards M more words.  In the above example these words
            will occupy word1 - word10.  In general M is the number of words
            necessary to fill all the previous columns we have.  M is
            calculated as follows:
                M = ( INT( total_number_of_word_columns / 2 ) - 1 ) * window_rows       ( if M &lt; 0 then M = 0 )

        where total_number_of_word_columns = INT( window_columns / WORD_COLUMN_LENGTH )

            If a word is longer than WORD_COLUMN_LENGTH-1, then we count it
            as two words.  For example, in the above example if word3 were
            longer than WORD_COLUMN_LENGTH-1, it would spill into the column
            for word13.  Thus this word effectively takes up two word spaces.

        7.  Call DIC_BROWSE_FILL to fill outstr with words beginning at lbptr.

---------------------------------------------------------------------------*/
void dic_browse_word( int inwdlen,
                      char *inwdbeg,
                      int window_columns,
                      int window_rows )
{
   /* NOTE: pages referred to are edx_dictionary page numbers of size DICPLN */
   int i, m, n;
   int low;             /* lower bound page # */
   int high;            /* upper bound page # */
   int target_len;      /* length of target word */
   int word_length;     /* word length */
   char *wdptr;         /* word pointer */
   char *wdend;         /* end of word pointer */
   char *dptr;          /* pointer into dictionary into word */
   char *tptr;          /* pointer into target_word */
   char *lbptr;         /* pointer into current word */
   char *lbbeg;         /* start here */
   char *endrange;

   /* STEP 1
      SETUP_DICWORD
      Move misspelled word to storage place, lowercase, blank pad to DICSWD chars */
   wdend = inwdbeg + inwdlen;                   /* wdend -&gt; char after last char of word */
   for ( wdptr = inwdbeg, i = 0;
         wdptr &lt; wdend;
         ++wdptr, ++i )
      target_word[i] = _tolower(*wdptr);

   target_word_len = inwdlen;
   for ( ; i &lt; dichead.indswd; ++i )            /* BLANK PAD TO INDSWD LENGTH */
      target_word[i] = SPACE;


/* STEP 2 */
/* SEARCH MAIN DICTIONARY FOR MATCH */
   binsrch_maindic( &low, &high, &target_word );

/* STEP 3
   LINEAR SEARCH DICTIONARY PAGES FOR BEST MATCH TO TARGET WORD
   Compare found word with target word for match of first n characters
   starting with n=target_word_length character and moving backwards
   towards front of word.  We do this because we already expect the first
   few characters of the word to match if we're looking in the right
   area of the dictionary.  (target_len = n)
*/
   endrange = dic.lexdba + high * dichead.dicpln;
   for ( lbbeg = dic.lexdba + low * dichead.dicpln; *lbbeg &gt; 31; ++lbbeg);      /* find a length-byte to start on */

/* lbbeg now points to length-byte */
/* Search for target-word, target-wor, target-wo, target-w, etc... until match */
   for (target_len = target_word_len;
        target_len &gt; 0;                                         /* (if n reaches 0 then dictionary doesn't have any words which start with this letter) */
        --target_len)
   {
      for( lbptr = lbbeg;                                       /* start here */
           lbptr &lt; endrange && *lbptr != 0x00;                  /* don't go past endrange or past end of lexical database */
           lbptr += *lbptr + 1)                                 /* save last word, move to next word */
      {
         if (*lbptr &gt;= target_len)                              /* word must be at least n chars long else no match */
         {
            for ( tptr = target_word + target_len -1,           /* start tptr at n'th char of target_word */
                  dptr = lbptr + target_len;                    /* start dptr at n'th char of word in dictionary */
                  tptr &gt;= target_word && *tptr == *dptr;        /* while chars match up to beginning of word */
                  --tptr, --dptr);                              /* move back a char */
            if (tptr &lt; target_word) goto match_found;           /* match found, exit loops */
         }/*test for match of first n chars*/
      }/*try all words in range [lbbeg:endrange]*/
   }/*try matching first n, n-1, n-2,... chars until match*/
   ++target_len;                                                /* (for loop decremented target_len before terminating loop) */
   lbptr = lbbeg;                                               /* (for loop advanced lbptr to next word before terminating loop) */
match_found:

/* STEP 4
   At this point we have lbptr pointing to first word in dictionary which
   matches first target_len chars.  (There is no word which matches first
   target_len + 1 chars).  Now find best closest match of next character
   for which there is no exact match.  Next character is either next
   character of same word or length-byte of next word (possibly NULL if
   we're at the end of the last word in the dictionary).  Search forward
   word by word until current dictionary word &gt; target_word (need only
   check first target_len+1 chars). */
   if (target_len &lt; target_word_len)                                    /* if we didn't get an exact match... */
   {
      lbbeg = lbptr;                                                    /* last lbptr */
      while ( lbptr &lt; endrange && *lbptr != 0x00                        /* don't go past endrange or past end of lexical database */
            &&
              string_compare( (int)*lbptr,                              /* length of dictionary word (may be less than 'length to match' below) */
                                  lbptr+1,                              /* start of dictionary word */
                             target_len+1,                              /* length to match */
                              target_word )                             /* word to match */
                 &lt;= 0 )
      {
         lbbeg = lbptr;                                                 /* save pointer to this word */
         lbptr += *lbptr + 1;                                           /* move to next word */
      }
      lbptr = lbbeg;                                            /* (while loop advanced lbptr to next word before terminating loop) */
   }

/* STEP 5
   At this point, lbptr points to length-byte of word we seek.  Count N
   words backwards not looking at word length.  (N = window_rows) */
   for ( n = window_rows; n &gt; 0; --n )
   {
      if (lbptr == dic.lexdba) break;           /* don't back up any more if we're at the beginning of the database */
      for( --lbptr;                             /* backup one char to get off length-byte */
           *lbptr &gt; 31;                         /*  then while not on length-byte */
           --lbptr );                           /*   move back a char */
   }


/* STEP 6
   Count M words backwards looking at word length
     if word length &gt;= word_column_length-1 then count as two words
     total_number_of_word_columns = INT( window_columns / WORD_COLUMN_LENGTH )
     M = ( total_number_of_word_columns/2 - 1 ) * window_rows
*/
   for ( m = ( (window_columns/WORD_COLUMN_LENGTH)/2 - 1 ) * window_rows;
         m &gt; 0                  /* Count M words backwards */
      && lbptr &gt; dic.lexdba;    /* check for at beginning of dictionary */
         --m)
      /* GO BACK ONE WORD (or WORD_COLUMN_LENGTH-1 chars, whichever comes first) */
      for ( word_length = 0, --lbptr;           /* Start with zero word_length, backup one char off length-byte */
            *lbptr &gt; 31                         /* While not found length-byte (separating words) */
         && word_length != WORD_COLUMN_LENGTH-1;/*  and word still fits in column */
            ++word_length, --lbptr );           /* Increment word_length, backup a char */

   /* IN CASE WE ENDED UP IN THE MIDDLE OF A LONG WORD, MOVE FORWARD TO
      THE FIRST LENGTH-BYTE.  (USUALLY WE'RE ALREADY ON IT AT THIS POINT) */
   for ( ; *lbptr &gt; 31; ++lbptr);               /* move forward to first length-byte (probably already on it) */

/* STEP 7
   NOW CALL DIC_BROWSE_FILL TO FILL OUTSTR WITH
   WORDS FROM DICTIONARY STARTING AT lbptr */
   dic_browse_fill( lbptr, window_columns, window_rows );
}

/*--------------------------------------------------------------------------

        .SUBTITLE DIC_BROWSE_FILL

 Functional Description:
        Fills OUTSTR with words from dictionary starting at dptr address
        into lexical database.

 Calling Sequence:
        dic_browse_fill( dptr_param, window_columns, window_rows );

 Argument inputs:
        dptr_param    - address in dictionary lexical database to start at.
                        This pointer MUST point to a length-byte (the byte
                        preceding each word giving the length of the word.
                        Identified by being less than ASCII 32.)
        window_columns - number of characters across window display (width) (by value)
        window_rows    - number of rows in window display to fill (height) (by value)

 Outline:
            Create temporary buffer for OUTSTR and fill it in.  We fill in
            the words starting with the first (left most) column, working our
            way down that column to the bottom.  Then proceeding with the
            next column until all the columns are filled.  For each word
            we calculate the offset into OUTSTR as follows:

        offset = row_number * nchars_per_row +
                        word_column_number * word_column_length

            where row_number goes from 0 to window_rows-1
            and word_column_number goes from 0 to number_of_word_columns-1
            with number_of_word_columns = INT( window_columns/WORD_COLUMN_LENGTH )

            Check to see if the word in the previous column is overflowing
            into the cell we were going to place our current word.  If so
            we move down the column to the next cell and try again.

            If working on the last (right most) column then we check to
            see if the word we are inserting is longer than will fit on
            the screen.  If so we truncate it and replace the last character
            with a "."

        EDX DUMP_DICTIONARY calls this to get the next word from the dictionary.
        It calls specifying rows=1, columns=0.  If columns=0 is specified,
        then one word (the next word in line) is placed in the output buffer
        and the output buffer length set to 32.

---------------------------------------------------------------------------*/
#define MAX_WORD_SIZE  32       /* maximum size of word in dictionary */

void dic_browse_fill( char *dptr_param, int window_columns, int window_rows )
{
   int word_column_number,
       row_number,
       offset,
       outbufsize,
       sig_array[3];
   char *dptr;
   char *outbuf;        /* output buffer */

/* CHECK THAT DPTR POINTS TO LENGTH-BYTE.  ERROR IF NOT. */
   if (*dptr_param &gt; 31)
   {
      edx_signal(1, byref(EDX__DBFDPTRLB) );
      fmtoutstr( 1, byref(EDX__DBFDPTRLB) );
      return;
   }

   dicptra = dptr = dptr_param;                 /* set DICPTRA (global) and DPTR (local) */
   outbufsize = window_rows * window_columns + MAX_WORD_SIZE;
   outbuf = (char *) malloc(outbufsize);        /* get a temporary buffer */
   memset( outbuf, ' ', outbufsize );           /* blank fill our OUTSTR buffer */

/* LOOP ACROSS ALL WINDOW WORD COLUMNS */
   word_column_number = 0;
   do                                           /* if window_columns=0 we do one word.  For dump_dictionary */
   {
      /* LOOP DOWN ALL WINDOW ROWS (in this word_column) */
      if (*dptr == 0x00) break;                 /* test for end of dictionary */
      for (row_number = 0;                      /* Start at first row of new column */
           *dptr != 0x00                        /* check for end of dictionary */
           && row_number &lt; window_rows;         /* loop down all window_rows */
           ++row_number )
      {
         offset = window_columns*row_number
               + word_column_number*WORD_COLUMN_LENGTH; /* offset into buffer */

         if (word_column_number == 0)                   /* if first column then we know slot is free */
         {
            memcpy((outbuf+offset),dptr+1,*dptr);       /* insert word into OUTSTR */
            dptr += *dptr+1;                            /* move to next word */
         }
         else
         {
            /* CHECK THAT SLOT IS EMPTY */
            if  ( *(outbuf+offset-1) == SPACE )         /* test previous character for blank */
            {                                           /* if not then previous word on this line */
                                                        /*  is too long.  We must move down to next column */
               /* ADD WORD TO OUT */
               if ( (word_column_number == window_columns/WORD_COLUMN_LENGTH - 1 )              /* is word_column = last_word_column? */
                    && (*dptr &gt; WORD_COLUMN_LENGTH) )                                   /* and length of current word longer than screen length left */
               {
                  memcpy((outbuf+offset),dptr+1,WORD_COLUMN_LENGTH-1);  /* insert word into OUTSTR */
                  *(outbuf+offset+WORD_COLUMN_LENGTH-1) = '.';          /* dot indicating word goes off edge of screen */
               }
               else
                  memcpy((outbuf+offset),dptr+1,*dptr); /* insert word into OUTSTR */
               dptr += *dptr+1;                         /* move to next word */
            }/*endif slot empty*/
         }/*endif first column*/
      }/* END LOOP DOWN ALL WINDOW ROWS in this word_column */
      ++word_column_number;
   } while( word_column_number &lt; window_columns/WORD_COLUMN_LENGTH );   /* END LOOP ACROSS ALL WINDOW WORD COLUMNS */


/* We drop out here when our OUTSTR buffer is full */
   dicptrz = dptr;                      /* set DICPTRZ */
   sig_array[0] = SS$_NORMAL;           /* (next line) if (window_rows*window_columns) = 0 then use MAX_WORD_SIZE (for use by DUMP_DICTIONARY) */
   sig_array[1] = ( (window_rows*window_columns) == 0 ? MAX_WORD_SIZE : (window_rows*window_columns) );
   sig_array[2] = outbuf;
   fmtoutstr( 3, &sig_array );
   free(outbuf);                                /* release temporary memory */
}

/*--------------------------------------------------------------------------

        .SUBTITLE SPELL_GUESS

 Functional Description:
        Guesses the spelling of misspelled word stored in DIC_LWA,DIC_LWL.
        Algorythm taken from the very popular Vassar Spelling Checker.
        With credit to Vassar where credit is due.

 Calling Sequence:
        spell_guess();

 Argument inputs:
        DIC_LWA - Address of misspelled word
        DIC_LWL - Length of misspelled word
        GMODE   - guess mode    (1=reversals,2=vowels,3=minus,4=plus,5=consonants,6=giveup)
        GOF     - guess column offset (character # in word working on)
        GSUBMODE- (char) guess submode (letter we're currently replacing with)

 Outputs:
        (retcode is placed in outstr by fmtoutstr)
        retcode=LIB$_NORMAL, outline="guessed word"
        here's another word to try, ask user if guessed word is what he ment.
     or
        retcode = SS$_ENDFILE, no more guesses.

 Outline:
        1.  Reversals   (test for transposed characters)
        2.  vowels      (test for wrong vowel used)
        3.  minus chars (test for extra character in word)
        4.  plus chars  (test for character missing from word)
        5.  consonants  (test for wrong character used)
        6.  give up     (give up)

---------------------------------------------------------------------------*/
#define GUSREV  1                               /* 1 = GUESS REVERSALS */
#define GUSVOL  2                               /* 2 = GUESS VOWELS */
#define GUSMIN  3                               /* 3 = GUESS MINUS */
#define GUSPLS  4                               /* 4 = GUESS PLUS */
#define GUSCON  5                               /* 5 = GUESS CONSONANTS */
#define GIVEUP  6                               /* 6 = GIVE UP */

void spell_guess()
{
   switch (gmode)                               /* GUESS MODE */
   {
      case GUSREV:                              /* 1 = GUESS REVERSALS */
           if (spell_gusrev()) return;          /* TRUE if guess word found. outstr set.  gcol, gmode, gsubmode hold our place for reentry */
           ++gmode;                             /* go to next guess mode */
           gof = gsubmode = 0;                  /* reset GOF and GSUBMODE */
                                                /* DROP THROUGH TO NEXT GUESS MODE */
      case GUSVOL:                              /* 2 = GUESS VOWELS */
           if (spell_gusvol()) return;          /* TRUE if guess word found. outstr set.  gcol, gmode, gsubmode hold our place for reentry */
           ++gmode;                             /* go to next guess mode */
           gof = gsubmode = 0;                  /* reset GOF and GSUBMODE */
                                                /* DROP THROUGH TO NEXT MODE: GUSMIN */
      case GUSMIN:                              /* 3 = GUESS MINUS */
           if (spell_gusmin()) return;          /* TRUE if guess word found. outstr set.  gcol, gmode, gsubmode hold our place for reentry */
           ++gmode;                             /* go to next guess mode */
           gof = gsubmode = 0;                  /* reset GOF and GSUBMODE */
                                                /* DROP THROUGH TO NEXT MODE: GUSPLS */
      case GUSPLS:                              /* 4 = GUESS PLUS */
           if (spell_guspls()) return;          /* TRUE if guess word found. outstr set.  gcol, gmode, gsubmode hold our place for reentry */
           ++gmode;                             /* go to next guess mode */
           gsubmode = 0;                        /* reset GSUBMODE */
           gof = 0;                             /* reset GOF */
                                                /* DROP THROUGH TO NEXT MODE: GUSCON */
      case GUSCON:                              /* 5 = GUESS CONSONANTS */
           if (spell_guscon()) return;          /* TRUE if guess word found. outstr set.  gcol, gmode, gsubmode hold our place for reentry */
                                                /* DROP THROUGH TO NEXT MODE: GIVEUP */
      case GIVEUP:                              /* 6 = GIVE UP */
      gmode = GUSREV;                           /* reset */
      gof = gsubmode = 0;                       /* reset GOF and GSUBMODE */
      fmtoutstr(1, &SS$_ENDOFFILE );            /* no more guesses */
   }
}


/*--------------------------------------------------------------------------*/

int spell_gusrev()
{
   int status;
   int sig_array[3];
   char guess_word[BUFLEN];
   char temp;

   /* Guess reversals.
      Copy word and transpose x with x+1 */
   while(gof &lt; dic_lwl-1)                               /* test for beyond end of word */
   {
      if (dic_lwa[gof] != dic_lwa[gof+1])               /* don't swap if characters are identical */
      {
         memcpy(guess_word,dic_lwa,dic_lwl);            /* copy over word */
         temp = guess_word[gof];                        /* swap chars */
         guess_word[gof] = guess_word[gof+1];
         guess_word[gof+1] = temp;
         status = dic_lookup_word( dic_lwl, guess_word );       /* see if word exists */
         if (status & STS$M_SUCCESS)
         {
            ++gof;                                      /* move to next character for reentry */
            sig_array[0] = status;                      /* retcode */
            sig_array[1] = dic_lwl;                     /* length of output string */
            sig_array[2] = guess_word;                  /* address of output string */
            fmtoutstr( 3, &sig_array );                 /* format output string */
            return(SUCCESS);                            /* return with string containing a correctly spelled word, status */
         }
      }
      ++gof;                                            /* move to next character */
   }
   return(FAILURE);                     /* no more guess words found */
}

int spell_gusvol()
{
   int status;
   int sig_array[3];
   char guess_word[BUFLEN];

   /* Guess vowel replacements.
      For each {a,e,i,o,u} replace with {a,e,i,o,u}
      GSUBMODE goes from 0-4 as letter replacement goes a,e,i,o,u */
   while(gof &lt; dic_lwl)                                 /* test for beyond end of word */
   {
      memcpy(guess_word,dic_lwa,dic_lwl);               /* copy over word */
      if (guess_word[gof] == 'a'
       || guess_word[gof] == 'e'
       || guess_word[gof] == 'i'
       || guess_word[gof] == 'o'
       || guess_word[gof] == 'u')
      {
         while(gsubmode &lt; 5)
         {
            switch (gsubmode)
            {
               case 0: guess_word[gof] = 'a'; break;    /* 1 = replace with an "a" */
               case 1: guess_word[gof] = 'e'; break;    /* 2 = replace with an "e" */
               case 2: guess_word[gof] = 'i'; break;    /* 3 = replace with an "i" */
               case 3: guess_word[gof] = 'o'; break;    /* 4 = replace with an "o" */
               case 4: guess_word[gof] = 'u'; break;    /* 5 = replace with an "u" */
               default:  return(EDX__GUSINTERR2);       /* return but don't signal (gsubmode may be 6 is OK) */
            }
            if (guess_word[gof] != dic_lwa[gof])        /* if we didn't replace vowel with same vowel */
            {
               status = dic_lookup_word( dic_lwl, guess_word ); /* see if word exists */
               if (status & STS$M_SUCCESS)
               {
                  ++gsubmode;                           /* set to guess next vowel for next time */
                  sig_array[0] = status;                /* retcode */
                  sig_array[1] = dic_lwl;               /* length of output string */
                  sig_array[2] = guess_word;            /* address of output string */
                  fmtoutstr( 3, &sig_array );           /* format output string */
                  return(SUCCESS);                      /* return with string containing a correctly spelled word, status */
               }/*endif(status);*/
            }/*endif(guess_word[gof]!=dic_lwa[gof]);*/
            ++gsubmode;                                 /* move to next vowel */
         }/*endwhile(gsubmode&lt;5)*/
         gsubmode=0;                                    /* reset gsubmode */
      }/*endif(guessword=aeiou*/
      ++gof;                                            /* move to next character */
   }/*endwhile(gof&lt;dic_lwl-1)*/
   return(FAILURE);                             /* no more guesses */
}

int spell_gusmin()
{
   int status;
   int sig_array[3];
   char guess_word[BUFLEN];

   /* Guess minus.  Test for extra character.
      Try eliding one character at a time */
   while(gof &lt; dic_lwl)                                         /* test for beyond end of word */
   {
      if (gof == 0 || dic_lwa[gof] != dic_lwa[gof-1])           /* skip if prev char = current char. The result would be the same */
      {                                                         /*  as last time.  (Also check gof==0 first) */
         memcpy(&guess_word[0],&dic_lwa[0],gof);                /* copy over word */
         memcpy(&guess_word[gof],&dic_lwa[gof+1],dic_lwl-(gof+1));/* shift GOF'th+1 to end of word left one */
         status = dic_lookup_word( dic_lwl-1, guess_word );     /* see if word exists */
         if (status & STS$M_SUCCESS)
         {
            ++gof;                              /* move to next char for reentry */
            sig_array[0] = status;              /* retcode */
            sig_array[1] = dic_lwl-1;           /* length of output string */
            sig_array[2] = guess_word;          /* address of output string */
            fmtoutstr( 3, &sig_array );         /* format output string */
            return(SUCCESS);                    /* return with string containing a correctly spelled word, status */
         }/*endif(status);*/
      }/*endif(not double char)*/
      ++gof;                                    /* move to next char */
   }/*endwhile(gof&lt;dic_lwl)*/
   return(FAILURE);                             /* no more guesses */
}


int spell_guspls()
{
   int status;
   int sig_array[3];
   char guess_word[BUFLEN];
   char guess_char;

   /* Guess plus.  Test if a letter is missing from word.  Add one letter anywhere in word. */
   /* GSUBMODE goes from 0-25 as letter replacement goes from a-z */
   while(gof &lt;= dic_lwl)                                        /* test for beyond end of word */
   {
      memcpy(&guess_word[0],&dic_lwa[0],gof);                   /* copy over word */
      memcpy(&guess_word[gof+1],&dic_lwa[gof],dic_lwl-gof);     /* shift GOF'th+1 to end of word left one */
      while(gsubmode &lt; 26)                                      /* test for GSUBMODE=25 (all letters of alphabet) */
      {
         guess_char = gsubmode + 'a';                           /* convert GSUBMODE={0-25} to ASCII {A-Z} (which is {65-90} */
         if (gof == 0 || guess_char != dic_lwa[gof-1])          /* if extra char being inserted = char it's infront of */
         {                                                      /*  then don't do it to avoid duplicates */
            guess_word[gof] = guess_char;                       /* insert missing letter */
            status = dic_lookup_word( dic_lwl+1, guess_word );  /* see if word exists */
            if (status & STS$M_SUCCESS)
            {
               ++gsubmode;                      /* set to try next char on reentry */
               sig_array[0] = status;           /* retcode */
               sig_array[1] = dic_lwl+1;        /* length of output string */
               sig_array[2] = guess_word;       /* address of output string */
               fmtoutstr( 3, &sig_array );      /* format output string */
               return(SUCCESS);                 /* return with string containing a correctly spelled word, status */
            }/*endif(status);*/
         }/*endif(not double char)*/
         ++gsubmode;                            /* try next char */
      }/*endwhile(gsubmode&lt;26)*/
      gsubmode=0;                               /* reset gsubmode */
      ++gof;                                    /* move to next char */
   }/*endwhile(gof&lt;dic_lwl)*/
   return(FAILURE);                             /* no more guesses */
}


int spell_guscon()
{
   int status;
   int sig_array[3];
   int isvowel;
   char guess_word[BUFLEN];
   char guess_char;

   /* Guess consonants.  Test for any one character wrong.
      Replace each character with every other character of the alphabet
      GSUBMODE goes from 0-25 as letter replacement goes from a-z */
   while(gof &lt; dic_lwl)                                 /* test for beyond end of word */
   {
      if (   (dic_lwa[gof] == 'a')                      /* if char is a vowel */
          || (dic_lwa[gof] == 'e')
          || (dic_lwa[gof] == 'i')
          || (dic_lwa[gof] == 'o')
          || (dic_lwa[gof] == 'u') )
         isvowel = TRUE;
      else
         isvowel = FALSE;
      while(gsubmode &lt; 26)                                      /* test for GSUBMODE=25 (all letters of alphabet) */
      {
         guess_char = gsubmode + 'a';                           /* convert GSUBMODE={0-25} to ASCII {A-Z} (which is {65-90} */
         if (   (guess_char != dic_lwa[gof])                    /* if overstrike char != original char */
             && ( !isvowel )                                    /* and char being replaced isn't a vowel */
             || (   (guess_char != 'a')                         /* or if char being replaced is a vowel */
                 && (guess_char != 'e')                         /* then do only if guess_char isn't a vowel */
                 && (guess_char != 'i')                         /* (otherwise we already tried it in spell_gusvol) */
                 && (guess_char != 'o')
                 && (guess_char != 'u') ) )
         {                                                      /*  or then don't do it to avoid duplicates */
            memcpy(guess_word,dic_lwa,dic_lwl);                 /* copy over word */
            guess_word[gof] = guess_char;                       /* overstrike with another letter */
            status = dic_lookup_word( dic_lwl, guess_word );    /* see if word exists */
            if (status & STS$M_SUCCESS)
            {
               ++gsubmode;                      /* set to try next char on reentry */
               sig_array[0] = status;           /* retcode */
               sig_array[1] = dic_lwl;          /* length of output string */
               sig_array[2] = guess_word;       /* address of output string */
               fmtoutstr( 3, &sig_array );      /* format output string */
               return(SUCCESS);                 /* return with string containing a correctly spelled word, status */
            }/*endif(status);*/
         }/*endif(not double char)*/
         ++gsubmode;                            /* try next char */
      }/*endwhile(gsubmode&lt;26)*/
      gsubmode=0;                               /* reset gsubmode */
      ++gof;                                    /* move to next char */
   }/*endwhile(gof&lt;dic_lwl)*/
   return(FAILURE);                             /* no more guesses */
}

/*---------------------------------------------------------------------------

        .SUBTITLE ACCEPTED_WORD_LIST

        SPELL_ACCEPT_WORD

 Functional Description:
        Constructs a balanced binary tree of words which the user has instructed
        us to accept as properly spelled.  The VMS library routine
        LIB$INSERT_TREE is used to build the tree.  DIC_LOOKUP_WORD uses
        the VMS library routine LIB$LOOKUP_TREE to search this tree for a
        match before declaring a word misspelled.

        The routines ALLOCATE_NODE and COMPARE_NODE are called by LIB$INSERT_TREE
        and LIB$LOOKUP_TREE.  ALLOCATE_NODE allocates memory and inserts
        the word to accept in the allocated memory.  COMPARE_NODE
        alphabetically compares the word stored in a given memory block
        with a given word to determine which comes first.

 Calling Sequence:
        spell_accept_word();

 Inputs:
        dic_lwa - (Global) Pointer to word to accept.  Set by DIC_LOOKUP_WORD.
        dic_lwl - (Global) Length of word to accept.  Set by DIC_LOOKUP_WORD.

*/

void spell_accept_word()
{
   int status;
   struct dsc$descriptor_s word_desc = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 };
   word_desc.dsc$w_length = dic_lwl;
   word_desc.dsc$a_pointer = dic_lwa;

   status = lib$insert_tree( &accept_tree, &word_desc, &0,
                   compare_node, allocate_node, &newnode, 0);
   fmtoutstr( 1, &status );
}
/*===========================================================================

        SAVE_CORRECTION

 Functional Description:
        Constructs a balanced binary tree of misspelled words and their
        corrections.  String #1 is the misspelled word and string #2 is
        the correct spelling.  The VMS library routine LIB$INSERT_TREE
        is used to build the tree.  After declaring a word misspelled,
        SPELL_TEXTLINE uses the VMS library routine LIB$LOOKUP_TREE to
        search this tree for a match.  If a match is found the correct
        spelling is returned to EDX.  EDX then asks the user if he
        wishes to make the correction.

        The routines ALLOCATE_NODE and COMPARE_NODE are called by LIB$INSERT_TREE
        and LIB$LOOKUP_TREE.  ALLOCATE_NODE allocates memory and inserts
        the word to accept in the allocated memory.  COMPARE_NODE
        alphabetically compares the word stored in a given memory block
        with a given word to determine which comes first.

 Calling Sequence:
        save_correction( inwds_desc );

 Inputs:
     inwds_desc - Descriptor of string containing misspelled word and correct
                  word.  The format of the string is misspelled word followed
                  by a single space followed by the correct word.

 Outline:
        1.  descriptors str1_desc & str1_desc are initialized.
        2.  inwds string is parsed:
                  first word is placed in str1
                  second word is placed in str2
            Leading whitespace is skipped and characters are lowercased
            in the process.  The string lengths are placed in the
            .dsc$w_length part of each string descriptor.
        3.  LIB$INSERT_TREE is called to add a new node to the tree.
---------------------------------------------------------------------------*/
void save_correction( struct dsc$descriptor_s *inwds_desc )
{
   int i;
   int status;
   char *inend;         /* points to one char after end of input string */
   char *wdptr;         /* points into string */
   char str1[BUFLEN];
   char str2[BUFLEN];
   struct dsc$descriptor_s str1_desc = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, str1 };
   struct dsc$descriptor_s str2_desc = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, str2 };

/* SKIP OVER LEADING SPACES AND TABS UNTIL WE FIND BEGINNING
   OF WORD OR END OF STRING.  ERROR IF STRING IS BLANK. */
   inend = inwds_desc-&gt;dsc$a_pointer + inwds_desc-&gt;dsc$w_length;        /* inend -&gt; char after last char of instring */
   for ( wdptr = inwds_desc-&gt;dsc$a_pointer;
         (wdptr &lt; inend) && isspace(*wdptr);                            /* Check for out of bounds before checking character */
         ++wdptr );
   if (wdptr == inend)                                                  /* was blank line */
   {
      fmtoutstr( 1, &FAILURE );
      return;
   }

/* MOVE 1ST WORD (MISSPELLED WORD) TO STR1 AND LOWERCASE */
   for ( i = 0;
         wdptr &lt; inend
         && i &lt; BUFLEN
         && !isspace(*wdptr);
         ++wdptr, ++i )
      str1[i] = _tolower(*wdptr);
   if (wdptr == inend || i == BUFLEN)                           /* error if end of string (no second word) */
   {                                                            /* or word1 overflowed buffer (too long) */
      fmtoutstr( 1, &FAILURE );
      return;
   }
   str1_desc.dsc$w_length = i;                                  /* length of 1st word */


/* MOVE 2ND WORD (CORRECTION) TO STR2 AND LOWERCASE */
   for ( ;                                      /* SKIP OVER WHITESPACE.  Look for beginning of second word */
         (wdptr &lt; inend) && isspace(*wdptr);    /* (Check for out of bounds before checking character) */
         ++wdptr );
   for ( i = 0;                                 /* NOW MOVE WORD TO STR2 */
         wdptr &lt; inend                          /* (Check for out of bounds before checking character) */
         && i &lt; BUFLEN
         && isascii(*wdptr);
         ++wdptr, ++i )
      str2[i] = _tolower(*wdptr);
   if (i == BUFLEN)                             /* error if word1 overflowed buffer (too long) */
   {                                            /* (Reaching end of input string is OK) */
      fmtoutstr( 1, &FAILURE );
      return;
   }
   str2_desc.dsc$w_length = i;                  /* length of 2nd word */


   status = lib$insert_tree( &savcor_tree, &str1_desc, &0,
                   compare_node, allocate_node, &newnode, &str2_desc);
   fmtoutstr( 1, &status );
}

/*---------------------------------------------------------------------------*/
/* The following routines are FOR DEBUGGING PURPOSES ONLY */
#if DEBUG

void print_accept_node(char *node)
{
   char *ptr, *end;
   ptr = (char *)(node+12);
   end = ptr + *(unsigned short *)(node+10);
   for ( ; ptr &lt; end; ++ptr)
      printf("%c",*ptr);
   printf("\n");
}

void print_savcor_node(char *node)
{
   char *ptr, *end;
   ptr = (char *)(node+12);
   end = ptr + *(unsigned short *)(node+10);
   for ( ; ptr &lt; end; ++ptr)    /* print the misspelled word */
      printf("%c",*ptr);
   printf("  ");
   end = ptr + 1 + *(unsigned char *)(ptr);
   for ( ; ptr &lt; end; ++ptr)    /* print the correct spelling */
      printf("%c",*ptr);
   printf("\n");
}

void traverse_savcor_tree()
   { lib$traverse_tree( &savcor_tree, print_savcor_node ); }

void traverse_accept_tree()
   { lib$traverse_tree( &accept_tree, print_accept_node ); }

#endif
/*===========================================================================

        ALLOCATE_NODE

 Functional Description:
        Allocates memory for a new node being added in a balanced binary
       tree by LIB$INSERT_TREE.  (alloc-rtn)

 Calling Sequence:
        Called by LIB$INSERT_TREE
          psudo-c call:
          status = allocate_node( symstr_desc, retadr, usrdat_desc );

 Argument inputs:
        number of arguments = 3
        symstr_desc (input) -
                    Pointer to string descriptor of string to insert in
                    this node.  (Descriptor class and type fields not used).
        retadr (output) -
                    Address to place starting address of block of
                    memory allocated and filled in.
        usrdat_desc [user data] -
                    If non-zero, address of string descriptor of string #2.

 Outputs:
        return status = LIB$_NORMAL if successfull
        return status = LIB$_INSVIRMEM if unsuccessfull

 Outline:
        1.  Memory is allocated and filled in as shown below:

 Map of node being allocated:
        -----------------------------------------( address of node placed in ret-adr )
        |              left link                |
        -----------------------------------------
        |              right link               |
        -----------------------------------------
        |  STRING#1 LENGTH  |      balance      |
        -----------------------------------------
        |               STRING#1                | 12
        |                   .                   |
        |                   .                   |
        -----------------------------------------
        |     String #2               |length #2| (optional string#2)
        |                   .         +---------|
        |                   .                   |
        -----------------------------------------

 The node header, consisting of the first 10 bytes of the node containing
 the left link, right link, and balance, is reserved for use by LIB$INSERT_TREE
 We fill in the string length, and the string itself, and we allocate enough
 memory to hold it all (12 bytes + length of string, +1 byte + length of
 string#2 if present).

If this is the tree of saved corrections, then the first string is a
misspelled word we came across earlier and string #2 is the correction
we made last time.  Upon finding this same misspelled word a second time,
EDX will ask if the user wants to make the same correction.
---------------------------------------------------------------------------*/
int allocate_node( struct dsc$descriptor_s *symstr_desc, char **retadr,
                   struct dsc$descriptor_s *usrdat_desc )
{
   int memlen;          /* length of memory to allocate */

   memlen = 12 + symstr_desc-&gt;dsc$w_length;     /* calculate length of memory = 12 for header + length of string (from descriptor) [(+1+length of string#2 if present)] */
   if (usrdat_desc != 0)                        /* test for string#2 */
      memlen += 1 + usrdat_desc-&gt;dsc$w_length;  /* [(+1+length of string#2 if present)] */

   *retadr = malloc( memlen );
   if (*retadr == 0)
   {
      edx_signal(1, byref(EDX__MEMERR) );
      fmtoutstr( 1, byref(EDX__MEMERR) );
      return(LIB$_INSVIRMEM);
   }

   *(unsigned short *)(*retadr+10) = symstr_desc-&gt;dsc$w_length;                 /* fill in length of string */
   memcpy(*retadr+12,symstr_desc-&gt;dsc$a_pointer,symstr_desc-&gt;dsc$w_length);     /* fill in string */
   if (usrdat_desc != 0)                        /* test for string#2 */
   {
      *(char *)(*retadr + 12 + symstr_desc-&gt;dsc$w_length)
             = usrdat_desc-&gt;dsc$w_length;       /* fill in length of string #2 */
      memcpy(*retadr + 12 + symstr_desc-&gt;dsc$w_length + 1,
             usrdat_desc-&gt;dsc$a_pointer,
             usrdat_desc-&gt;dsc$w_length);        /* fill in string #2 */
   }
   return(LIB$_NORMAL);
}
/*===========================================================================

        COMPARE_NODE

 Functional Description:
        Compares string to string contained in a given node.  Returns
        &gt;0, =0, &lt;0 for string GTR,EQL,LSS than given node.

 Calling Sequence:
        Called by LIB$INSERT_TREE:
          psudo-c call:
          status = compare_node(symstr_desc, treenode, usrdat_desc );

 Argument inputs:
        number of arguments = 3
        symstr_desc (input) - Address of string descriptor of string to
                              compare with given node.  (Descriptor class
                              and type fields not used).
        treenode (input) - Address of node to compare with string.  The format
                           of a node is shown in the memory map below.
        usrdat_desc [user-data] (not used) -
                           The address of a descriptor, or 0.

 Outputs:
        status = &gt;0 if string &gt; node
                 =0 if string = node
                 &lt;0 if string &lt; node

 Map of tree node:
        -----------------------------------------
        |              left link                | 00
        -----------------------------------------
        |              right link               | 04
        -----------------------------------------
        |   STRING LENGTH   |      balance      | 08
        -----------------------------------------
        |                STRING                 | 0C (=decimal 12)
        |                   .                   |
        |                   .                   |
        -----------------------------------------

---------------------------------------------------------------------------*/
int compare_node( struct dsc$descriptor_s *symstr_desc, char *treenode,
                  struct dsc$descriptor_s *usrdat_desc )
{
   return string_compare( symstr_desc-&gt;dsc$w_length,            /* length of symstr */
                          symstr_desc-&gt;dsc$a_pointer,           /* address of symstr */
                          *(unsigned short *)(treenode+10),     /* length of node string */
                          (char *)(treenode+12) );              /* address of node string */
}
/*===========================================================================

        .SUBTITLE ADD_PERSDIC

        ADD_PERSDIC

 Functional Description:
        Adds the current unrecognised word to the user's personal dictionary.

 Calling Sequence:
        spell_accept_word();

 Global Inputs:
        DIC_LWA - Address of word to accept.  Set by DIC_LOOKUP_WORD.
        DIC_LWL - Length of word to accept.  Set by DIC_LOOKUP_WORD.

 Outline:
        1.  Open user's personal dictionary.  If file does not exist,
            it is created.
        2.  Add new word to end of file.
        3.  Close file.

---------------------------------------------------------------------------*/

void add_persdic()
{
/* HOW TO INITIALIZE THIS ONLY ONCE? */
   int status;
   int sig_array[6];
   struct FAB addpdicfab;
   struct RAB addpdicrab;
   struct NAM addpdicnam;

   /* OPEN THE USER'S PERSONAL DICTIONARY FILE */
   /* Initialize ADDPDICNAM */
   addpdicnam = cc$rms_nam;                             /* initialize NAM */
   addpdicnam.nam$b_rss = NAM$C_MAXRSS;                 /* Expanded file name string size */

   /* Initialize ADDPDICFAB */
   addpdicfab = cc$rms_fab;                             /* initialize FAB */
   addpdicfab.fab$l_fna = (char *) "EDXPERSDIC";        /* file name address */
   addpdicfab.fab$b_fns = LENGTH("EDXPERSDIC");         /* file name length  */
   addpdicfab.fab$l_dna = (char *) "SYS$LOGIN:EDXPERSDIC.DAT";  /* default file name address */
   addpdicfab.fab$b_dns = LENGTH("SYS$LOGIN:EDXPERSDIC.DAT");   /* default file name length  */
   addpdicfab.fab$l_nam = &addpdicnam;                  /* NAM block address */
   addpdicfab.fab$b_fac = FAB$M_PUT;                    /* File Access options */
   addpdicfab.fab$b_shr = FAB$M_SHRGET;                 /* share options */
   addpdicfab.fab$l_fop = FAB$M_CIF;                    /* options = create if nonexist */

   /* Initialize ADDPDICRAB */
   addpdicrab = cc$rms_rab;                             /* initialize RAB */
   addpdicrab.rab$l_fab = &addpdicfab;                  /* address of FAB */
   addpdicrab.rab$l_rop = RAB$M_EOF;                    /* Position to end of file for append operation */

   status = sys$create(&addpdicfab);                    /* Open user's personal dictionary file */
   if ( !(status & STS$M_SUCCESS) )
   {
      sig_array[0] = EDX__PERSDICERR;                   /* error opening personal dictionary */
      sig_array[1] = 2;                                 /* 2 FAO args */
      sig_array[2] = addpdicnam.nam$b_rsl;              /* filename size */
      sig_array[3] = addpdicnam.nam$l_rsa;              /* filename address */
      sig_array[4] = status;                            /* Error status */
      sig_array[5] = 0;                                 /* 0 FAO args */
      edx_signal( 6, sig_array );
   }

   if ( (status == RMS$_CREATED) )
   {
      sig_array[0] = EDX__CREPERSDIC;                   /* created personal dictionary */
      sig_array[1] = 2;                                 /* 2 FAO args */
      sig_array[2] = addpdicnam.nam$b_rsl;              /* filename size */
      sig_array[3] = addpdicnam.nam$l_rsa;              /* filename address */
      edx_signal( 4, sig_array );
   }

   status = sys$connect(&addpdicrab);                   /* Connect to user's personal dictionary file */
   if ( !(status & STS$M_SUCCESS) )
   {
      sig_array[0] = EDX__PERSDICERR;                   /* error opening personal dictionary */
      sig_array[1] = 2;                                 /* 2 FAO args */
      sig_array[2] = addpdicnam.nam$b_rsl;              /* filename size */
      sig_array[3] = addpdicnam.nam$l_rsa;              /* filename address */
      sig_array[4] = status;                            /* Error status */
      sig_array[5] = 0;                                 /* 0 FAO args */
      edx_signal( 6, sig_array );
   }

   addpdicrab.rab$l_rbf = dic_lwa;
   addpdicrab.rab$w_rsz = dic_lwl;
   status = sys$put(&addpdicrab);                       /* Add word to user's personal dictionary */
   if (status & STS$M_SUCCESS)
   {
      sig_array[0] = EDX__WORDADD;                      /* added word to personal dictionary file */
      sig_array[1] = 4;                                 /* 2 FAO args */
      sig_array[2] = dic_lwl;                           /* word size */
      sig_array[3] = dic_lwa;                           /* word address */
      sig_array[4] = addpdicnam.nam$b_rsl;              /* filename size */
      sig_array[5] = addpdicnam.nam$l_rsa;              /* filename address */
      edx_signal( 6, sig_array );
   }
   else
      edx_signal( addpdicrab.rab$l_sts, addpdicrab.rab$l_stv );         /* signal error */
   sys$close(&addpdicfab);                              /* close file */
}

/*----------------------------------------------------------------------------

        .SUBTITLE DUMP_COMMONWORDS

        DUMP_COMMONWORDS

 Functional Description:
        Returns as a single string the list of common words as stored in
        the EDX dictionary database file.

 Calling Sequence:
        dump_commonwords();

 Outputs:
        OUTSTR - String of common words, blank separated.

Outline:
        1.  Scratch memory is allocated.
        2.  The string of common words is copied to the scratch memory.
        3.  The length-bytes preceeding each word are replaced with space
            characters.
        4.  The resulting string consisting of the common words, blank
            separated, is sent to fmtoutstr().
        5.  The scratch memory is deallocated.
*/
void dump_commonwords()
{
   int wlen;            /* word length */
   char *outbuf;        /* output buffer */
   char *lbptr;         /* pointer to length-byte of current word */
   char *endrange;
   int sig_array[3];
   outbuf = (char *) malloc(dichead.cwdlen);    /* get a temporary buffer */
   memcpy(outbuf,cmnwdsptr,dichead.cwdlen);     /* copy common words to buffer */

   endrange = outbuf + dichead.cwdlen;          /* end of commonwords */
   lbptr = outbuf;                              /* start at beginning of common words */
   while (lbptr &lt; endrange)                     /* until we reach the end of the common word list */
   {
      wlen = *lbptr;                            /* length of this word */
      *lbptr = ' ';                             /* change length-byte to space */
      lbptr += wlen + 1;                        /* move to next word */
   }

   sig_array[0] = SUCCESS;                      /* return status */
   sig_array[1] = dichead.cwdlen;               /* length */
   sig_array[2] = outbuf;                       /* address */
   fmtoutstr( 3, &sig_array );

   free(outbuf);                                /* free memory */
}
/*

==============================================================================
        EDX SORT
==============================================================================

Sort Routines:
  EDX_SORT                      !Main entry.  Parses R6
  SORT_PREPARSE                 !Preparse SORT command
  SORT_PASSFILES                !Pass filenames for file sort
  SORT_POSTPARSE                !Finish parsing SORT command
  SORT_DO_FILE                  !Do file sort
  SORT_RELEASE_REC              !Give record to sort when using record sort
  SORT_RETURN_REC               !Get record when using record sort

        .SUBTITLE EDX_SORT

 Functional Description:
        Sorts using either file sort or record sort.  This routine uses
        the VMS Sort/Merge (SOR) Utility Routines as described in the
        VAX/VMS Utilities Routines Reference Manual.

 Calling Sequence:
        edx_sort();

 Argument inputs:
    incodel = Code describing subfunction to perform (low word of INCODE)
              1.  Preparse command line
              2.  Pass files and do sort (for file sort)
              3.  Postparse command line
              4.  Pass a record to sort.  (Repeat until all records passed)
              5.  Do record sort
              6.  Receive a record in sorted order.  (Repeat until all records received)
              7.  Cleanup

 Usage:
   Either file sort or record sort is used.

   With file sort, you pass it the name of a file on disk to sort,
   it sorts the file creating a new file and returns to you the name of
   the new file created.

   With record sort, you pass it individual records (a record is a line
   of text from the buffer), passing it one record per call until all
   records have been passed.  The records are sorted, and then returned
   to you one record per call, in sorted order, until all records have
   been returned to you.

 Which sorting method to use:
    File sort can handle all situations.  It is the method of choice
    when a large number of records are to be sorted, since it is faster
    to write a large buffer to disk than to pass it one line at a time
    to and from sort, and there is no limit to the length of line which
    can be sorted.  However, the file sort method takes a minimum of
    a couple seconds because of time it takes to create a temporary
    file on disk and then delete it.

    Record sort is suitable when a small number of lines are to be
    quickly sorted, and the lines are &lt; 132 characters in length (SRT_MAXLRL).
    As the number of records to sort increases, there reaches a point
    where it becomes faster to use the file sort method instead.

 Sequence of calls for performing file sort:
       1.  Call n=1 'Preparse command line' passing the SORT command
           line to be parsed.  Returns indicating if SORT BUFFER,
           SORT RANGE, HELP, or error in sort command.
       2.  Call n=2 'Pass files and do sort' passing it name of file to sort.
           Returns name of sorted file created.

 Sequence of calls for performing record sort:
       1.  Call n=1 'Preparse command line' passing the SORT command
           command line to be parsed.  Returns indicating if SORT BUFFER,
           SORT RANGE, HELP, or error in sort command.
       2.  Call n=3 'Postparse command line'
       3.  Call n=4 'Pass a record to sort' passing it one record from
           the buffer.  Repeat this call until all records have been
           passed.
       4.  Call n=5 'Do record sort' to perform the actual sort
       5.  Call n=6 'Receive a record in sorted order'.  Returns one
           record.  Repeat until all records have been passed back in
           sorted order.
       6.  Call n=7 'Cleanup' to free up memory allocated by sort.

----------------------------------------------------------------------
 Internal sequence for performing file sort:
        1.  EDX calls SORT_PREPARSE passing it the SORT command line.
            SORT_PREPARSE parses the command line.  If there is an error
            it returns with error status.  Otherwise it returns indicating
            wheter SORT BUFFER or SORT RANGE was specified on command line.
        2.  EDX writes a temporary file to disk to be sorted.
        3.  EDX calls SORT_PASSFILES passing it the name of the temporary
            file to be sorted.  SORT_PASSFILES generates a filename for
            the output file, and then calls SOR$PASS_FILES passing the names
            of the input file and the output file.
        4.  SORT_POSTPARSE is called.  It extracts the information from CLI
            about the previously parsed SORT command, and calls SOR$BEGIN_SORT.
        5.  SORT_DO_FILE is called, which calls SOR$SORT_MERGE to do the actual
            sort.  It also calls SOR$END_SORT to clean up afterwards.

 Internal sequence for performing record sort:
        1.  EDX calls SORT_PREPARSE passing it the SORT command line.
            SORT_PREPARSE parses the command line.  If there is an error
            it returns with error status.  Otherwise it sets OUTSTR indicating
            wheter SORT BUFFER or SORT RANGE was specified on command line.
        2.  SORT_POSTPARSE is called.  It extracts the information from CLI
            about the previously parsed SORT command, and calls SOR$BEGIN_SORT.
           Return to user with OUTSTR previously set.
        3.  SOR$RELEASE_REC is called for n=4.
        4.  SOR$SORT_MERGE is called for n=5.
        5.  SOR$RETURN_REC is called for n=6.
        6.  SOR$END_SORT is called for n=7.

 Note:
        The SOR$M_... symbols are defined in the SYS$LIBRARY:SORTSHR.EXE
        shareable image.  The linker should resolve these symbols
        automatically, as part of its search through the images in the
        system shareable image library SYS$LIBRARY:IMAGELIB.OLB.

------------------------------------------------------------------------------*/
#define SRT_MAXLRL 132          /* Maximum length of line we will support for record sort */
                                /* There is a tradeoff here between memory used and speed achieved */
                                /* A larger MAXLRL causes SORT to allocate more memory */
#define ASCENDING 0             /* Used in key_buffer */
#define DESCENDING 1            /* Used in key_buffer */
#define FILE_SORT 0             /* sort_type = FILE_SORT */
#define RECORD_SORT 1           /* sort_type = RECORD_SORT */

void edx_sort()
{
   int incodel;
   int status;

   switch (*incode_ptr)
   {
      case SORT_FILE_INIT:     sort_preparse();                 /* Sort initialize for file sort */
                               return;

      case SORT_FILE_DO:       status = sort_passfiles();       /* Do file sort */
                               if (status & STS$M_SUCCESS)      /* Success passing filenames */
                               {
                                  status = sort_postparse(FILE_SORT);   /* using file sort */
                                  if (status & STS$M_SUCCESS)           /* Success sorting */
                                  {
                                     sort_do_file();
                                     return;                            /* Normal return */
                                  }
                               }
                               fmtoutstr( 1, &status );                 /* else return bad status */
                               return;


      case SORT_RECORDS_INIT:  status = sort_postparse(RECORD_SORT);    /* Sort initialize for record sort */
                               fmtoutstr( 1, &status );
                               return;

      case SORT_RECORDS_PASS:  sort_release_rec();              /* Pass a record to sort */
                               return;

      case SORT_RECORDS_DO:    status = sor$sort_merge();       /* Do record sort */
                               if ( !(status & STS$M_SUCCESS) ) /* Error sorting */
                                  edx_signal(1,&status);
                               fmtoutstr( 1, &status );
                               return;

      case SORT_RECORDS_RECV:  sort_return_rec();               /* Receive a record in sorted order */
                               return;

      case SORT_FINISH:        status = sor$end_sort();         /* Cleanup record sort */
                               fmtoutstr( 1, &status );
                               return;

      default:  edx_signal(1, byref(EDX__UNKNCODE) ); return;
   }
}

/*---------------------------------------------------------------------------

        SORT_PREPARSE

 Functional Description:
        Parses the command line for correctness and returns indicating
        if BUFFER or RANGE was specified.  We hold off on examining the
        rest of the results from the parsed command line until after
        SOR$PASS_FILES has been called (if file sort is being used), then
        the rest is done in SORT_POSTPARSE.

 Calling Sequence:
        sort_preparse();

 Argument inputs:
        INSTR = Command line to be parsed

 Outputs:
        OUTSTR - return value indicating domain to be sorted.
           = 0 no verb specified
           = 1 BUFFER was specified
           = 2 RANGE was specified
           = 3 HELP was specified
---------------------------------------------------------------------------*/

void sort_preparse()
{
   int status;
   $CONST_DESCRIP(domain_desc,"DOMAIN");
   char valbuf[8];
   $DESCRIPTOR( valbuf_desc, valbuf );

/* Clean up any outstanding sort */
   sor$end_sort();                                      /* clean up incase previous unfinished sort was active */

/* The command is parsed. */
   status = cli$dcl_parse(&instr_desc, &edx_commands);  /* Parse input string */
   if ( !(status & STS$M_SUCCESS) )                     /* Error parsing input string */
   {
      fmtoutstr( 1, &FAILURE );
      return;
   }

/* Now see if BUFFER, RANGE, or HELP was specified */
   cli$get_value( &domain_desc, &valbuf_desc, &valbuf_desc.dsc$w_length );
   if ( string_compare( valbuf_desc.dsc$w_length, valbuf_desc.dsc$a_pointer,
                        valbuf_desc.dsc$w_length, "BUFFER" ) == 0 )             /* User may specify B, BU, BUF, BUFF, BUFFE, BUFFER */
      fmtoutstr( 1, &3 );                                               /* BUFFER specified */
   else if ( string_compare( valbuf_desc.dsc$w_length, valbuf_desc.dsc$a_pointer,
                             valbuf_desc.dsc$w_length, "RANGE" ) == 0 ) /* User may specify R, RA, RAN, RANG, RANGE */
      fmtoutstr( 1, &2 );                                               /* RANGE specified */
   else if ( string_compare( valbuf_desc.dsc$w_length, valbuf_desc.dsc$a_pointer,
                             valbuf_desc.dsc$w_length, "HELP" ) == 0 )
      fmtoutstr( 1, &4 );                                               /* HELP specified */
   else
      fmtoutstr( 1, &0 );                               /* no verb specified */
}

/*---------------------------------------------------------------------------

        SORT_PASSFILES

 Functional Description:
        Calls SOR$PASS_FILES, passing the name of the input filename to
        be sorted, and the output filename to create.  The input filename
        of the file to sort is passed to us in INSTR.  The output filename
        is generated by us here.  It is of the form "EDX_TEMPSORT00000000.SRT"
        where the 00000000 is the current process's PID in hexadecimal.
        We use the current process's PID as part of the filename to help
        make the filename unique.

 Calling Sequence:
        status = sort_passfiles();

 Argument inputs:
        INSTR = Input filename to pass to SOR$PASS_FILES

 Outputs:
        OUTSTR = Output filename we generated, with return status of 1

 Outline:
    The output filename is of the form: EDX_TEMPSORT00000000.SRT
    where the 00000000 is replaced by the process's PID number.
    A. The process's PID is determined by calling SYS$GETJPI
    B. 00000000 of the output filename is replaced by the
       process's PID number.  (OTS$CVT_L_TZ)
    C. SOR$PASS_FILES is called, passing the input filename from INSTR,
       and the output filename we created.
    D. If success, then our generated output filename is copied to OUTSTR
       (for temporary storage).

*/
int sort_passfiles()
{
   int status;
   int pid;
   int sig_array[3];
   char outfile[] = { "EDX_TEMPSORT00000000.SRT" };
   $DESCRIPTOR(outfile_desc,outfile);
   struct dsc$descriptor_s  pid_desc = { 8, DSC$K_DTYPE_T, DSC$K_CLASS_S, &outfile[12] };
   item_list_3 itemlist[2] = { {  4, JPI$_PID, &pid,  0  },
                               {  0,        0,    0,  0  }  };

/* THE PROCESS'S PID IS DETERMINED BY CALLING SYS$GETJPIW */
   status = sys$getjpiw( 0, 0, 0, itemlist, 0, 0, 0 );
   if ( !(status & STS$M_SUCCESS) )
   {
     edx_signal( 1, &status );
     fmtoutstr( 1, &status );
     return(status);
   }

/* 00000000 of the output filename is replaced by the process's PID number. */
   status = ots$cvt_l_tz( &pid, &pid_desc, 8 );
   if ( !(status & STS$M_SUCCESS) )
   {
     edx_signal( 1, &status );
     fmtoutstr( 1, &status );
     return(status);
   }

/* SOR$PASS_FILES is called, passing the input filename from INSTR,
   and the output filename we created. */
   status = sor$pass_files( &instr_desc, &outfile_desc );
   if ( !(status & STS$M_SUCCESS) )
   {
     edx_signal( 1, &status );
     fmtoutstr( 1, &status );
     return(status);
   }

/* If success, then our generated output filename is copied to OUTSTR
   (for temporary storage). */
   sig_array[0] = SUCCESS;
   sig_array[1] = outfile_desc.dsc$w_length;
   sig_array[2] = outfile_desc.dsc$a_pointer;
   fmtoutstr( 3, &sig_array );
   return(status);
}

/*-----------------------------------------------------------------------------

        SORT_POSTPARSE

 Functional Description:
        Extracts information from CLI about the previously parsed SORT command.
        Builds and item list of this information and calls SOR$BEGIN_SORT.

 Calling Sequence:
        status = sort_postparse( sort_type );

 Argument inputs:
        sort_type - (0 or 1 by value)
                    0 = using file sort.
                    1 = using record sort.

 Outputs:
        Status.  Signaled if bad.

 Implicit:
        It is assumed a SORT command line was preveously parsed by
        SORT_PREPARSE.

 Outline:
    1.  Key_Buffer is filled in depending upon qualifiers found
        in the command string.  The key_buffer itemlist is filled
        from the top down, with a new item added whenever a /KEYn
        sort key qualifier is found.
    1a.    KEY.--- qualifiers are examined
    1b.    KEYn.--- qualifiers are examined with n = 1-9
    2.  If no keys were given
           then create 1 key with default values:  ascending, offset=0,
                                                   keysize = {maximum}
           and check for qualifiers /DESCENSING, /REVERSE, /START.
    3. Check for qualifiers /STABLE, /EBCDIC, /MULTINATIONAL, /NODUPLICATES
    4. SOR$BEGIN_SORT is called to initialize the sort

Memory Map:
  Structure of key_buffer:
   We allocate enough storage for 10 keys, (/KEY and /KEY1 - /KEY9)
   the first word is the number of keys actually filled in.

        ---------------------
        |number of keys used|
        -----------------------------------------
        |       order       |     type          | \
        -----------------------------------------   key 0
        |       length      |     offset        | /
        -----------------------------------------
        |       order       |     type          | \
        -----------------------------------------   key 1
        |       length      |     offset        | /
        -----------------------------------------
        .                                       .
        .                                       .
        .                                       .
        -----------------------------------------
        |       order       |     type          | \
        -----------------------------------------   key 9
        |       length      |     offset        | /
        -----------------------------------------

 Notes:
        The SOR$M_... symbols are defined in the SYS$LIBRARY:SORTSHR.EXE
        shareable image.  The linker should resolve these symbols
        automatically, as part of its search through the images in the
        system shareable image library SYS$LIBRARY:IMAGELIB.OLB.
*/

int sort_postparse( int sort_type )
{
   int i;
   int position;
   int size;
   int sort_options;
   int lrl;                             /* longest record length */
   int status;
   short int value_len;
   char value[255];
   char keyn[] = { "KEYn" };
   char keyn_position[] = { "KEYn.POSITION" };
   char keyn_size[] = { "KEYn.SIZE" };
   char keyn_descending[] = { "KEYn.DESCENDING" };
   char keyn_reverse[] = { "KEYn.REVERSE" };
   $DESCRIPTOR(value_desc,value);
   $DESCRIPTOR(keyn_desc,keyn);
   $DESCRIPTOR(keyn_position_desc,keyn_position);
   $DESCRIPTOR(keyn_size_desc,keyn_size);
   $DESCRIPTOR(keyn_descending_desc,keyn_descending);
   $DESCRIPTOR(keyn_reverse_desc,keyn_reverse);

   struct
   {
      short int numkeys;
      struct
      {
         short type;
         short order;
         short offset;
         short length;
      } key[10];
   } key_buffer;

   i = 0;                                       /* initialize */

/* CHECK FOR KEY QUALIFIER */
   if ( cli$present(s_descrip("KEY")) & STS$M_SUCCESS )                 /* Test for KEY, then test for KEY1 - KEY9 */
   {
      key_buffer.key[0].type = DSC$K_DTYPE_T;                   /* Fill in Type is text characters */

      /* CHECK FOR KEY.DESCENDING AND/OR KEY.REVERSE (IDENTICAL) */
      if (  ( cli$present(s_descrip("KEY.DESCENDING")) & STS$M_SUCCESS )        /* Test for KEY.DESCENDING */
          ||( cli$present(s_descrip("KEY.REVERSE"))    & STS$M_SUCCESS ) )      /* or for REVERSE (identical function) */
         key_buffer.key[0].order = DESCENDING;                                  /* set key descending (same as reverse) */
      else
         key_buffer.key[0].order = ASCENDING;                   /* else set key ascending */

      /* GET KEY.POSITION */
      cli$get_value( s_descrip("KEY.POSITION"), &value_desc, &value_len );
      lib$cvt_dtb( value_len,                   /* CONVERT STRING TO NUMBER.  length of string to convert, */
                   value_desc.dsc$a_pointer,    /* string by reference, */
                   &position );                 /* result by reference (longword). */
      key_buffer.key[0].offset = position - 1;  /* Convert position to offset */

      /* GET KEY.SIZE */
      cli$get_value( s_descrip("KEY.SIZE"), &value_desc, &value_len );
      lib$cvt_dtb( value_len,                   /* CONVERT STRING TO NUMBER.  length of string to convert, */
                   value_desc.dsc$a_pointer,    /* string by reference, */
                   &size );                     /* result by reference (longword). */
      key_buffer.key[0].length = size;          /* Convert longword to word */

      i = 1;                                    /* count of # of keys we have accumulated so far */
   }

/* CHECK FOR KEYn QUALIFIERS */
   for ( keyn[3] = '1'; keyn[3] &lt;= '9'; ++keyn[3] )     /* keyn counts from "KEY1" to "KEY9" */
   {

      /* CHECK FOR KEYn QUALIFIER */
      if ( cli$present(&keyn_desc) & STS$M_SUCCESS )    /* Test for KEY1 - KEY9 */
      {
         key_buffer.key[i].type = DSC$K_DTYPE_T;        /* Fill in Type is text characters */

         /* CHECK FOR KEYn.DESCENDING AND/OR KEYn.REVERSE (IDENTICAL) */
         keyn_descending[3] = keyn[3];                                  /* make KEYn_DESCENDING into KEY1_DESCENDING - KEY9_DESCENDING */
         keyn_reverse[3] = keyn[3];                                     /* make KEYn_REVERSE into KEY1_REVERSE - KEY9_REVERSE */
         if (  ( cli$present(&keyn_descending_desc) & STS$M_SUCCESS )   /* Test for KEY.DESCENDING */
             ||( cli$present(&keyn_reverse_desc)    & STS$M_SUCCESS ) ) /* or for REVERSE (identical function) */
            key_buffer.key[i].order = DESCENDING;                       /* set key descending (same as reverse) */
         else
            key_buffer.key[i].order = ASCENDING;                        /* else set key ascending */

         /* GET KEYn.POSITION */
         keyn_position[3] = keyn[3];                            /* make KEYn_POSITION into KEY1_POSITION - KEY9_POSITION */
         cli$get_value( &keyn_position_desc, &value_desc, &value_len );
         lib$cvt_dtb( value_len,                                /* CONVERT STRING TO NUMBER.  length of string to convert, */
                      value_desc.dsc$a_pointer,                 /* string by reference, */
                      &position );                              /* result by reference (longword). */
         key_buffer.key[i].offset = position - 1;               /* Convert position to offset */

         /* GET KEYn.SIZE */
         keyn_size[3] = keyn[3];                                /* make KEYn_SIZE into KEY1_SIZE - KEY9_SIZE */
         cli$get_value( &keyn_size_desc, &value_desc, &value_len );
         lib$cvt_dtb( value_len,                                /* CONVERT STRING TO NUMBER.  length of string to convert, */
                      value_desc.dsc$a_pointer,                 /* string by reference, */
                      &size );                                  /* result by reference (longword). */
         key_buffer.key[i].length = size;                       /* Convert longword to word */

         ++i;                                                   /* Increment key_buffer index */
      }
   }/*end for keyn[3]='1'-'9'*/


/* DONE PARSING KEYS.  NOW APPLY DEFAULTS (if we didn't get anything) */
   if (i == 0)                                                  /* IF NO SORT QUALIFIERS */
   {                                                            /* THEN USE DEFAULT VALUES */
      key_buffer.key[0].type = DSC$K_DTYPE_T;                   /* Fill in Type is text characters */
      key_buffer.key[0].order = ASCENDING;                      /* sort defaults to ascending */
      key_buffer.key[0].offset = 0;                             /* sort starts by default at offset 0 */
      key_buffer.key[0].length = 65535;                         /* key length defaults to 65535 (as big as we can go) */

      if (  ( cli$present(s_descrip("DESCENDING")) & STS$M_SUCCESS )    /* Test for /DESCENDING */
          ||( cli$present(s_descrip("REVERSE"))    & STS$M_SUCCESS ) )  /* or for /REVERSE (identical function) */
         key_buffer.key[0].order = DESCENDING;                  /* set key descending (same as reverse) */
      else
         key_buffer.key[0].order = ASCENDING;                   /* else set key ascending */

      if ( cli$present(s_descrip("START")) & STS$M_SUCCESS )    /* Test for /START */
      {
         cli$get_value( s_descrip("START"), &value_desc, &value_len );  /* Check for /START */
         lib$cvt_dtb( value_len,                                /* CONVERT STRING TO NUMBER.  length of string to convert, */
                      value_desc.dsc$a_pointer,                 /* string by reference, */
                      &position );                              /* result by reference (longword). */
         key_buffer.key[0].offset = position - 1;               /* Convert position to offset */
      }
      i = 1;                                                    /* Count of number of keys is 1 */
   }/*end if i=0*/


/* NOW PARSE FOR SORT_OPTIONS */
   sort_options = SOR$M_NOSIGNAL;                                       /* start with this by default */
   if ( cli$present(s_descrip("STABLE")) & STS$M_SUCCESS )              /* Test for /STABLE */
      sort_options |= SOR$M_STABLE;                                     /* Set stable bit */

   if ( cli$present(s_descrip("EBCDIC")) & STS$M_SUCCESS )              /* Test for /EBCDIC */
      sort_options |= SOR$M_EBCDIC;                                     /* Set EBCDIC bit */

   if ( cli$present(s_descrip("MULTINATIONAL")) & STS$M_SUCCESS )       /* Test for /MULTINATIONAL */
      sort_options |= SOR$M_MULTI;                                      /* Set MULTINATIONAL bit */

   if ( cli$present(s_descrip("DUPLICATES")) == CLI$_NEGATED )          /* Test for /NODUPLICATES */
      sort_options |= SOR$M_NODUPS;                                     /* Set noduplicates bit */

   if (sort_type & RECORD_SORT)
      lrl = SRT_MAXLRL;                         /* length of longest line if using record sort */
   else
      lrl = 0;                                  /* if using file sort leave LRL=0 */

/* CALL SOR$BEGIN_SORT */
   key_buffer.numkeys = i;                      /* number of keys we have defined */
   status = sor$begin_sort( &key_buffer, &lrl, &sort_options );
   if ( !(status & STS$M_SUCCESS) )
      edx_signal(1,&status);                    /* signal error */
   return(status);                              /* return sor$begin_sort status */
}

/*----------------------------------------------------------------------------

        SORT_DO_FILE

 Functional Description:
        Perform actual sort.

 Calling Sequence:
        sort_do_file();

 Implicit inputs:
        It is assumed that SOR$BEGIN_SORT has already been called
       (by SORT_POSTPARSE).

 Outputs:
        OUTSTR has already been set by SORT_PASSFILES with success code
        of 1 and string containing ouput filename.  This will be used as
        our return string unless an error occurs here, in which case
        we remake OUTSTR with our own error information.

 Outline:
        1.  SOR$SORT_MERGE is called to sort file creating new output file
        2.  SOR$END_SORT is called to clean up
*/
void sort_do_file()
{
   int status;

   status = sor$sort_merge();                   /* DO SORT */
   if ( !(status & STS$M_SUCCESS) )
   {
     edx_signal( 1, &status );
     fmtoutstr( 1, &status );
   }
   sor$end_sort();
}                                       /* OUTSTR was set previously */

/*--------------------------------------------------------------------------

        SORT_RELEASE_REC

 Functional Description:
        Pass a record to SORT when using record sort.  Calls SOR$RELEASE_REC

 Calling Sequence:
        sort_release_rec();

 Inputs:
        INSTR - record being released to SORT

 Outputs:
        Status in OUTSTR: 0 = error
                          1 = success
                          2 = line too long

 SRT_MAXLRL is maximum length of record we can do via record sort
*/
void sort_release_rec()
{
   int status;

   status = sor$release_rec( &instr_desc );
   if (status & STS$M_SUCCESS)
     fmtoutstr( 1, &SUCCESS );
   else
   {
      if (status == SOR$_BAD_LRL)                       /* Check for line too long status */
         fmtoutstr( 1, &2 );                            /* Set return status to 2 (line too long) */
      else
      {
         edx_signal( 1, &status );
         fmtoutstr( 1, &FAILURE );
      }
   }
}
/*---------------------------------------------------------------------------

        SORT_RETURN_REC

 Functional Description:
        Returns a record when using record sort

 Calling Sequence:
        sort_return_rec();

 Outputs:
        OUTSTR - returned record.

----------------------------------------------------------------------------*/
void sort_return_rec()
{
   int status;
   int sig_array[3];
   char buffer[SRT_MAXLRL];
   $DESCRIPTOR(buffer_desc,buffer);

   status = sor$return_rec( &buffer_desc, &buffer_desc.dsc$w_length );  /* Get next string (in sorted order) */
   if ( !(status & STS$M_SUCCESS) && (status != SS$_ENDOFFILE) )
      edx_signal( 1, &status );

   sig_array[0] = status;                       /* retcode */
   sig_array[1] = buffer_desc.dsc$w_length;     /* length of output string */
   sig_array[2] = buffer_desc.dsc$a_pointer;    /* address of output string */
   fmtoutstr( 3, &sig_array );                  /* format output string */
}
/*

==============================================================================
        LOCK FILES
==============================================================================
Files may be locked to prevent others from modifying or using tha file.
If you are making changes to a file and you don't want anyone else to
edit that file while you're making changes to it, you can lock that file.
Files are locked by opening them with noshare attributes.

A singularly linked list of locked files is kept.  locked_files_head is the
beginning of this list and points to the first node in the list or is NULL
if there are no locked files.

    Each node in the list contains:
      1. A pointer to the next node (or NULL if this is the last node in the list)
      2. A FAB block identifying the file (required by $RMS)
      3. A NAM block giving the full filename of the file
      4. An expanded_file_name buffer (attached to the NAM block)
      5. A resultant_file_name buffer (attached to the NAM block)


        .SUBTITLE LOCK_FILE

 Functional Description:
        This routine locks a file to prevent others from editing that file
        by opening that file with a noshare attribute.

 Calling Sequence:
        lock_file();

Inputs:
        instr_desc = Name of file to lock (by descriptor).

Outputs:
        OUTSTR = return status

 Outline:
        1.  A block of memory of struct 'locked_file' is allocated
            and filled in.
        2.  If the file is successfully opened the new block of
            memory is added to the end of the 'locked_files' list
            pointed to by locked_files_head.

-----------------------------------------------------------------------------*/

void lock_file()
{
   int status,
       sig_array[6];
   struct locked_file *locked_files_ptr,
                      *newnodeptr,
                      *prelnkptr,
                      *curlnkptr;

   /* Allocate memory for NEWNODE block */
   newnodeptr = (struct locked_file *) malloc ( sizeof(struct locked_file) );
   if (newnodeptr == 0) { edx_signal(1, byref(EDX__MEMERR) ); return; }

   /* Initialize FAB block */
   newnodeptr-&gt;fab = cc$rms_fab;
   newnodeptr-&gt;fab.fab$b_shr = FAB$M_NIL;
   newnodeptr-&gt;fab.fab$l_nam = &newnodeptr-&gt;nam;
   newnodeptr-&gt;fab.fab$l_fna = instr_desc.dsc$a_pointer;
   newnodeptr-&gt;fab.fab$b_fns = instr_desc.dsc$w_length;

   /* Initialize NAM block */
   newnodeptr-&gt;nam = cc$rms_nam;
   newnodeptr-&gt;nam.nam$l_esa = newnodeptr-&gt;expanded_file_name;
   newnodeptr-&gt;nam.nam$b_ess = NAM$C_MAXRSS;
   newnodeptr-&gt;nam.nam$l_rsa = newnodeptr-&gt;resultant_file_name;
   newnodeptr-&gt;nam.nam$b_rss = NAM$C_MAXRSS;

   /* open the file */
   status = sys$open( &newnodeptr-&gt;fab );
   if (status & STS$M_SUCCESS)
   {
      /* insert newnode at the end of the tree */
      newnodeptr-&gt;next = 0;
      if (locked_files_head == 0)
         locked_files_head = newnodeptr;
      else
      {
         for ( locked_files_ptr = locked_files_head;    /* Go to end of linked list of locked_files */
               locked_files_ptr-&gt;next != 0;
               locked_files_ptr = locked_files_ptr-&gt;next );
         locked_files_ptr-&gt;next = newnodeptr;
      }

      /* signal success */
      sig_array[0] = EDX__LOCKED;               /* Successfully locked file */
      sig_array[1] = 2;                         /* two FAO arguments */
      sig_array[2] = newnodeptr-&gt;nam.nam$b_rsl; /* Resultant filename size */
      sig_array[3] = newnodeptr-&gt;nam.nam$l_rsa; /* Address of resultant filename string */
      edx_signal( 4, sig_array );               /* Signal success message */
      fmtoutstr( 1, &SUCCESS );                 /* format return status */
   }
   else
   {  /* ELSE ERROR OPENING FILE.  SIGNAL ERROR, RELEASE MEMORY */
      if (   (status == RMS$_FLK)                                               /* If the error was 'file locked by another user' */
          && (srch_locked_files(&prelnkptr,&curlnkptr) == EDX__LOCKED) )        /* and we are the ones who have it locked */
      {
         sig_array[0] = EDX__ALK;
         sig_array[1] = 2;                              /* two FAO arguments */
         sig_array[2] = curlnkptr-&gt;nam.nam$b_esl;       /* Expanded filename size */
         sig_array[3] = curlnkptr-&gt;nam.nam$l_esa;       /* Address of expanded filename string */
         edx_signal( 4, sig_array );                    /* Signal message */
      }
      else
      {
         sig_array[0] = EDX__NOLOCK;                    /* file not locked */
         sig_array[1] = 2;                              /* two FAO arguments */
         sig_array[2] = newnodeptr-&gt;nam.nam$b_esl;      /* Expanded filename size */
         sig_array[3] = newnodeptr-&gt;nam.nam$l_esa;      /* Address of expanded filename string */
         sig_array[4] = status;                         /* error code */
         sig_array[5] = 0;                              /* zero FAO arguments for error */
         edx_signal( 6, sig_array );                    /* Signal message */
      }/*endif we already have file locked*/

#if DEBUG
      /* DEBUG.  Fill memory we're about to release with 'FF' making it unusable. */
      /*         Then make sure this node has been removed from the list. */
      { int i;  char *p;   struct locked_file *ptr;
        for (i=0, p=newnodeptr; i &lt; sizeof(struct locked_file); ++i, ++p) *p = 0xFF;
        if (locked_files_head != 0)
           for ( ptr = locked_files_head; ptr-&gt;next != 0; ptr = ptr-&gt;next );
      }
#endif

      free(newnodeptr);                                 /* release memory */
      fmtoutstr( 1, &FAILURE );                         /* format return status */
   }/*endif status=sys$open(&newnodeptr-&gt;fab)*/
}

/*----------------------------------------------------------------------------

        .SUBTITLE UNLOCK_FILE

 Functional Description:
        This procedure unlocks the specified file locked by routine LOCK_FILE
        by closing it and deallocating the memory for the locked_file BLOCK.

 Calling Sequence:
   unlock_file();

 Inputs:
        instr_desc = Name of file to unlock (by descriptor).

 Outputs:
        OUTSTR = {SUCCESS=1, FAILURE=0}

 Outline:
        1.  We search the linked list locked_files_head for the specified
            filename.  If found we close the file and remove the block
            from the linked list.  If not found we signal the error.

---------------------------------------------------------------------------*/
void unlock_file()
{
   int status;
   int sig_array[6];
   int outfile_len;
   char outfile[NAM$C_MAXRSS];
   struct locked_file *prelnkptr,       /* Previous Locked_files Block */
                      *curlnkptr;       /* Block Containing Filename */

   /* Find matching locked_file BLOCK for INSTR filename */
   status = srch_locked_files( &prelnkptr, &curlnkptr );/* Search for BLOCK containing filename */
   if (status == EDX__LOCKED)                           /* If successful then prelnk = Address of previous locked_files BLOCK (possibly zero) */
   {
      /* Found match.  Close file and return memory */
      /* curlnkptr = Address of locked_file BLOCK to close */
      /* prelnkptr = Address of previous locked_file BLOCK (maybe 0) */
      status = sys$close( &curlnkptr-&gt;fab );            /* close file */
      if (status & STS$M_SUCCESS)
      {
         sig_array[0] = EDX__UNLOCKED;                  /* Signal success message */
         sig_array[1] = 2;                              /* two FAO arguments */
         sig_array[2] = curlnkptr-&gt;nam.nam$b_rsl;       /* Expanded filename size */
         sig_array[3] = curlnkptr-&gt;nam.nam$l_rsa;       /* Address of expanded filename string */
         edx_signal( 4, sig_array );                    /* Signal message */
         if (prelnkptr != 0)
            prelnkptr-&gt;next = curlnkptr-&gt;next;          /* prevlnkptr points to previous BLOCK (may be 0 if no previous BLOCK) */
         else
            locked_files_head = curlnkptr-&gt;next;        /* was no previous link.  Set header to point to next BLOCK */

#if DEBUG
      /* DEBUG.  Fill memory we're about to release with 'FF' making it unusable. */
      /*         Then make sure this node has been removed from the list. */
      { int i;  char *p;   struct locked_file *ptr;
        for (i=0, p=curlnkptr; i &lt; sizeof(struct locked_file); ++i, ++p) *p = 0xFF;
        if (locked_files_head != 0)
           for ( ptr = locked_files_head; ptr-&gt;next != 0; ptr = ptr-&gt;next );
      }
#endif

         free(curlnkptr);                       /* free up memory */
         fmtoutstr( 1, &SUCCESS );
         return;
      }
      else   /* else error closing file.  Signal error and return */
      {
         edx_signal(1,&status);                 /* signal error */
         fmtoutstr( 1, &FAILURE );
         return;
      }

   }  /* else status != EDX_LOCKED */
   else if (status == EDX__NOTLOCKED)           /* Test for not found failure */
   {
      /* File Not Found in linked list. */
      /* Came to end of linked list.  No match found. */
      /* Reparse input filename not using physical device names to get
         concealed logical names. */
      status = edx_parse( NAM$C_MAXRSS,                 /* output buffer length (by value) */
                          outfile,                      /* output buffer (char array by reference) */
                          &outfile_len,                 /* result length (by reference) */
                          instr_desc.dsc$w_length,      /* input length (by value) */
                          instr_desc.dsc$a_pointer,     /* input (by value) */
                          FALSE );                      /* don't use physical device names (by value) */
      sig_array[0] = EDX__NOTLOCKED;
      sig_array[1] = 2;                         /* two FAO arguments */
      sig_array[2] = outfile_len;               /* Expanded filename size */
      sig_array[3] = outfile;                   /* Address of expanded filename string */
      edx_signal( 4, sig_array );               /* Signal message */
      fmtoutstr( 1, &FAILURE );                 /* format return status */
      return;
   }
   else  /* else error was something else.  Signal error and return. */
   {
      edx_signal(1,&status);
      fmtoutstr( 1, &FAILURE );                 /* format return status */
      return;
   }
}

/*----------------------------------------------------------------------------

        .SUBTITLE EDX_CKFILK

 Functional Description:
        This procedure searches the linked list of filenames locked by
        LOCK_FILE for a specified filename.  If the file is found it
        returns #1 in RETCODE to TPU.  If the file is not found it returns
        #0 in RETCODE to TPU.

 Calling Sequence:
        edx_ckfilk();

 Inputs:
        instr_desc = Name of file to check for (by descriptor).

 Outputs:
        OUTSTR = success (file found in locked_list),
                 failure (file not found in locked_list).

 Outline:
        1.  Call SRCH_LOCKED_FILES.  It does all the work.
            We just check the return status and set RETCODE accordingly.
-------------------------------------------------------------------------*/

void edx_ckfilk()
{
   int status;
   struct locked_file *prelnkptr,
                      *curlnkptr;

   status = srch_locked_files(&prelnkptr,&curlnkptr);   /* Search for BLOCK containing filename */
   if (status == EDX__LOCKED)
     fmtoutstr( 1, &SUCCESS );
   else if (status == EDX__NOTLOCKED)
     fmtoutstr( 1, &FAILURE );
   else
     fmtoutstr( 1, &status );
}

/*-------------------------------------------------------------------------

        .SUBTITLE SRCH_LOCKED_FILES

 Functional Description:
        This procedure searches the linked list of filenames locked by
        LOCK_FILE for a specified filename.

 Calling Sequence:
        struct locked_file *prelnkptr, *curlnkptr;
        status = srch_locked_files( &prelnkptr, &curlnkptr );

 Inputs:
        instr_desc = Name of file to check for (by descriptor).

 Returns:
        status = EDX__LOCKED if successful,
                 EDX__NOTLOCKED if failure,
                 or error status code.
        prelnkptr = Base address of previous LOCKED_FILES BLOCK
        curlnkptr = Base address of current LOCKED_FILES BLOCK which
                    has target filename.

 Outline:
        1.  The input filename is parsed to give a full filename
        2.  The filename of the first LOCKED_FILES BLOCK is parsed
        3.  A comparison of the two filenames is made
            If they match we've found our BLOCK
            If they don't match we try the next BLOCK
            If we come to the end of the linked list with no match we
             return with the error EDX__NOTLOCKED

            The target filename is matched with each filename in the linked
            list only up to the target filename length.  The target filename
            might not specify a version number.  If multiple versions of the
            same file are locked, and no version to unlock is specified,
            we will unlock the first matching filename we come across, which
            might be a version different from the default the user wanted
            to get.

            I have a vague memory that this was done on purpose to make
            unlocking files easier when you didn't know the version number.

Programming notes:
   a = prelnkptradr;            ! Address of prelnkptr.  (value of prelnkptradr)
   b = *prelnkptradr;           ! Value of prelnkptr.  (address of blk)
   c = (*prelnkptradr)-&gt;next;   ! c = blk.next
   d = &prelnkptradr;           ! Address of {address of prelnkptr} in call
                                !   frame argument list.  (SHOW SYMBOL/ADDRESS
                                !   prelnkptradr).

---------------------------------------------------------------------------*/
int srch_locked_files( struct locked_file **prelnkptradr,       /* pointer to a pointer to a structure of type locked_file */
                       struct locked_file **curlnkptradr )
{
   int status;
   int target_filename_length;
   int block_filename_length;
   char target_filename[NAM$C_MAXRSS];
   char block_filename[NAM$C_MAXRSS];

   /* Parse target filename */
   status = edx_parse( NAM$C_MAXRSS,                    /* output buffer length (by value) */
                       target_filename,                 /* output buffer (char array by reference) */
                       &target_filename_length,         /* output length (by reference) */
                       instr_desc.dsc$w_length,         /* input length (by value) */
                       instr_desc.dsc$a_pointer,        /* input buffer (char array by reference) */
                       TRUE );                          /* use physical device names (by value) */
   if ( !(status & STS$M_SUCCESS) ) return(status);     /* Else return error */

   /* Go through linked list of locked file BLOCKS */
   *curlnkptradr = locked_files_head;                   /* pointer to structure of type locked_file */
   *prelnkptradr = 0;                                   /* pointer to structure of type locked_file = NULL */
   while (*curlnkptradr != 0)
   {
      status = edx_parse( NAM$C_MAXRSS,                         /* Parse output filespec (length of output buffer by value) */
                          block_filename,                       /* out buffer (char array by reference) */
                          &block_filename_length,               /* result length (by reference) */
                          (*curlnkptradr)-&gt;nam.nam$b_rsl,       /* input length (by value) */
                          (*curlnkptradr)-&gt;nam.nam$l_rsa,       /* input buffer (char array by reference) */
                          TRUE );                               /* use physical device names (by value) */

      if (strncmp(target_filename,block_filename,target_filename_length) == 0)  /* found match */
         return( EDX__LOCKED );                         /* normal exit */
      *prelnkptradr = *curlnkptradr;                    /* go to next BLOCK in linked list */
      *curlnkptradr = (*curlnkptradr)-&gt;next;
   }
   return( EDX__NOTLOCKED );                            /* if we drop out the bottom */
}                                                       /*  then block not found */
                                                        /*  *prelnkptradr points to last block in linked list */
                                                        /*  *curlnkptradr = 0 */
/*---------------------------------------------------------------------------

        .SUBTITLE EDX_PARSE

 Functional Description:
        This routine accepts a filespec as input and parses it returning
        the full file specification using physical device names.

 Calling Sequence:
      status = edx_parse( output_filename_buffer_length,
                          output_filename_buffer,
                          output_filename_length,
                          input_filename_buffer_length,
                          input_filename_buffer,
                          pdn );

 Argument inputs:
        int  output_filename_buffer_length,     = (read)  length of output buffer
        char *output_filename_buffer,           = (write) address of output buffer
        int  output_filename_length,            = (write) length of resulting filename (only chars up to this length are valid)
        int  input_filename_buffer_length,      = (read)  length of input buffer
        char *input_filename_buffer,            = (read)  address of input buffer
        pdn                    = (read)  If TRUE then use physical device names.

 Outputs:
       status = Parse status

 Outline:
        1.  FAB and NAM blocks are initialized
        2.  The filespec is parsed
-------------------------------------------------------------------------*/

int edx_parse( int  output_filename_buffer_length,      /* read, by value */
               char *output_filename_buffer,            /* write, char array by reference */
               int  *output_filename_length,            /* write, by reference (has to be) */
               int  input_filename_buffer_length,       /* read, by value */
               char *input_filename_buffer,             /* read, char array by reference */
               int  pdn )                               /* read, by value.  Logical: use physical device names */
{
   int status;
   struct NAM nam;
   struct FAB fab;

   /* Initialize FAB block */
   fab = cc$rms_fab;
   fab.fab$l_nam = &nam;
   fab.fab$l_fna = input_filename_buffer;
   fab.fab$b_fns = input_filename_buffer_length;

   /* Initialize NAM block */
   nam = cc$rms_nam;
   nam.nam$l_esa = output_filename_buffer;
   nam.nam$b_ess = output_filename_buffer_length;
   nam.nam$b_nop = NAM$M_SYNCHK;
   if (pdn) nam.nam$b_nop |= NAM$M_NOCONCEAL;

   /* Parse the filename */
   status = sys$parse( &fab );                  /* Parse the filename */
   *output_filename_length = nam.nam$b_esl;     /* resultant expanded filename length (convert byte to int) */
   return( status );                            /* sys$parse status */
}
/*
==============================================================================
        MISCELLANEOUS
==============================================================================

        .SUBTITLE EDX_SETDEF

 Functional Description:
        This procedure changes a user's default directory.

 Calling Sequence:
        edx_setdef();

 Argument inputs:
        INSTR = Address of descriptor of string containing new directory to go to

 Outline:
        1.  The FAB and NAM blocks are initialized
        2.  The filespec is parsed
        3.  The node and disk are extracted and SYS$DISK is defined
        4.  Call SYS$SETDDIR
        5.  Check return status and signal if error

*/
void edx_setdef()
{
   int status;
   int length;
   struct NAM nam;
   struct FAB fab;
   char output_filename[BUFLEN];
   $DESCRIPTOR(output_filename_desc,output_filename);
   $CONST_DESCRIP(sys$disk_desc,"SYS$DISK");
   struct dsc$descriptor_s disk_desc = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 };

   /* Initialize FAB block */
   fab = cc$rms_fab;
   fab.fab$l_nam = &nam;
   fab.fab$l_fna = instr_desc.dsc$a_pointer;
   fab.fab$b_fns = instr_desc.dsc$w_length;

   /* Initialize NAM block */
   nam = cc$rms_nam;
   nam.nam$l_esa = output_filename_desc.dsc$a_pointer;
   nam.nam$b_ess = output_filename_desc.dsc$w_length;

   /* Parse the filename */
   status = sys$parse( &fab );                  /* Parse the filename */
   if ( !(status & STS$M_SUCCESS) )
   {
      edx_signal( 1, &status );
      fmtoutstr( 1, &FAILURE );
      return;
   }

   /* Set default disk by defining SYS$DISK as NODE::DISK: */
   disk_desc.dsc$w_length = length = nam.nam$b_node + nam.nam$b_dev;    /* Length of NODE::DISK: */
   if (length &gt; 0)                                                      /* otherwise DISK not specified */
   {
      disk_desc.dsc$a_pointer = nam.nam$l_node;
      status = lib$set_logical( &sys$disk_desc, &disk_desc );
      if ( !(status & STS$M_SUCCESS) )
      {
         edx_signal( 1, &status );
         fmtoutstr( 1, &FAILURE );
         return;
      }
   }

   /* Set default directory */
   status = sys$setddir( &instr_desc, 0, 0 );
   if (status & STS$M_SUCCESS)
      fmtoutstr( 1, &SUCCESS );
   else
   {
      edx_signal( 1, &status );
      fmtoutstr( 1, &FAILURE );
   }
}

/*---------------------------------------------------------------------------

        .SUBTITLE SET LOGICAL

 Functional Description:
        Defines a logical name.  The logical name is created in supervisor
        mode and placed in the LNM$PROCESS table.

 Calling Sequence:
        set_logical();

 Global inputs:
    instr_desc = Input string descriptor.  String is of the form:

                    "log-nam value"

It should be a substring of the full DCL type command "DEFINE log-nam value".

        log-nam = Logical name to be defined or redefined.
        value   = Value to be given to the logical name.

 Outline:
        1.  The input string is parsed.  A descriptor for the substring
            "log-nam" and a descriptor for the substring "value" are made.
        2.  LIB$SET_LOGICAL is called to define the logical name.
            The return status checked and signaled if there is an error.

------------------------------------------------------------------------------*/

void set_logical()
{
   int status;
   struct dsc$descriptor_s lognam_desc = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 };
   struct dsc$descriptor_s  value_desc = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 };
   char *p, *end;

   /* PARSE OFF LOG-NAM */
   end = instr_desc.dsc$a_pointer + instr_desc.dsc$w_length;
   for ( p = instr_desc.dsc$a_pointer; p &lt; end && *p != SPACE; ++p );   /* Find space character - End of "log-nam" */
   lognam_desc.dsc$w_length  = p - instr_desc.dsc$a_pointer;            /* Length of "log-nam" */
   lognam_desc.dsc$a_pointer = instr_desc.dsc$a_pointer;

   /* PARSE OFF VALUE */
   for ( ; p &lt; end && *p == SPACE; ++p );                               /* Find first non-space character - Beginning of "value" */
   value_desc.dsc$w_length  = end - p;                                  /* Length of "value" */
   value_desc.dsc$a_pointer = p;                                        /* Now points to beginning of VALUE
                                                                           (or one past end of string in which
                                                                            case length should be 0 */

   status = lib$set_logical( &lognam_desc, &value_desc );               /* Call LIB$SET_LOGICAL */
   if ( status & STS$M_SUCCESS)
      fmtoutstr( 1, &SUCCESS );
   else
   {
      /* Process error in creating logical name */
      edx_signal( 1, &status );
      fmtoutstr( 1, &FAILURE );
   }
}
/*---------------------------------------------------------------------------

        .SUBTITLE SET_SYMBOL

 Functional Description:
        Creates a DCL symbol.

 Calling Sequence:
        set_symbol();

 Global inputs:
    instr_desc = Input string descriptor.  String is of the form:

                 "symbol-name0equivalence0tblind"

                 where 0 represents an ascii 00 character.

        symbol-name = Name to be defined or redefined.
        expression  = Expression to be given to the symbol.
        tblind      = Indicator of the table which will contain the defined
                      symbol.  1=local, 2=global.  (See LIB$SET_SYMBOL)

 Outline:
        1.  The input string is parsed.  A descriptor for the substring
            "symbol-name", a descriptor for the substring "expression",
            and the value of tblind is extracted.

        2.  LIB$SET_SYMBOL is called to create the DCL symbol.
            The return status is checked and signaled if there is an error.

---------------------------------------------------------------------------*/
void set_symbol()
{
   struct dsc$descriptor_s symnam_desc = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 };
   struct dsc$descriptor_s expres_desc = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 };
   char *p, *end;
   int tblind;
   int status;

   /* PARSE OFF SYMBOL-NAME */
   end = instr_desc.dsc$a_pointer + instr_desc.dsc$w_length;
   symnam_desc.dsc$a_pointer = instr_desc.dsc$a_pointer;                /* Starting address of "symbol-name" */
   for ( p = symnam_desc.dsc$a_pointer; p &lt; end && *p != 0x00; ++p );   /* Find first null in string - End of "symbol-name" */
   symnam_desc.dsc$w_length  = p - symnam_desc.dsc$a_pointer;           /* Length of "symbol-name" */

   /* PARSE OFF EQUIVALENCE */
   ++p;                                                                 /* Skip over NULL */
   expres_desc.dsc$a_pointer = p;                                       /* Starting address of "equivalence" */
   for ( ; p &lt; end && *p != 0x00; ++p );                                /* Find first non-space character - Beginning of "value" */
   expres_desc.dsc$w_length  = p - expres_desc.dsc$a_pointer;           /* Length of "equivalence" */

   /* PARSE OFF TBLIND */
   ++p;                                                                 /* Skip over NULL */
   if (p &gt;= end)
   {
      /* fail with internal error.  tblind not specified. */
/*      edx_signal( 1, &status ); */
      fmtoutstr( 1, &FAILURE );
      return;
   }
   tblind = *p - '0';   /* convert character digit to integer value */

   /* Call LIB$SET_SYMBOL */
   status = lib$set_symbol( &symnam_desc, &expres_desc, &tblind );
   if (status & STS$M_SUCCESS)
   {
      fmtoutstr( 1, &SUCCESS );
   }
   else
   {
      /* Process error in setting symbol */
      edx_signal( 1, &status );
      fmtoutstr( 1, &FAILURE );
   }
}
/*---------------------------------------------------------------------------

        .SUBTITLE SHOW LOGICAL

 Functional Description:
        Translates a logical name

 Calling Sequence:
        show_logical();

 Global inputs:
    instr_desc  = Logical name to translate (by descriptor).

 Outputs:
    outstr_desc = translation of logical name (by descriptor).

 Outline:
        1.  Itemlist for call to SYS$TRNLNM is initialized
        2.  SYS$TRNLNM is called to obtain logical name translation

-----------------------------------------------------------------------------*/

void show_logical()
{
   int status;
   unsigned short retlen;
   int sig_array[3];
   char buffer[BUFLEN];
   $CONST_DESCRIP(lnm_table,"LNM$FILE_DEV");
   item_list_3 itemlist[2] = { { BUFLEN, LNM$_STRING, buffer, &retlen },
                               {      0,           0,      0,       0 } };

   status = sys$trnlnm( &LNM$M_CASE_BLIND,
                        &lnm_table,
                        &instr_desc,
                        0,
                        itemlist);              /* Translate logical name */
   if (status & STS$M_SUCCESS)
   {
      sig_array[0] = SUCCESS;
      sig_array[1] = retlen;
      sig_array[2] = buffer;
      fmtoutstr( 3, &sig_array );
   }
   else
   {
     edx_signal( 1, &status );
     fmtoutstr( 1, &FAILURE );
   }
}

/*-----------------------------------------------------------------------------

        .SUBTITLE SHOW SYMBOL

 Functional Description:
        Translates a DCL symbol

 Calling Sequence:
        show_symbol();

 Global inputs:
    instr_desc  = Symbol name to translate (by descriptor).

 Outputs:
    outstr_desc = translation of symbol (by descriptor).

 Outline:
    1.  LIB$GET_SYMBOL is called to obtain the symbol translation

---------------------------------------------------------------------------*/
void show_symbol()
{
   int status;
   int tblind;
   int sig_array[3];
   char buffer[BUFLEN];
   $DESCRIPTOR(buffer_desc,buffer);

   status = lib$get_symbol( &instr_desc,
                            &buffer_desc,
                            &buffer_desc.dsc$w_length,
                            &tblind);   /* Translate DCL symbol */
   if (status & STS$M_SUCCESS)
   {
      sig_array[0] = SUCCESS;
      sig_array[1] = buffer_desc.dsc$w_length;
      sig_array[2] = buffer;
      fmtoutstr( 3, &sig_array );       /* Copy translation to output */
   }
   else
   {
     edx_signal( 1, &status );
     fmtoutstr( 1, &FAILURE );
   }
}
/*-----------------------------------------------------------------------------

        .SUBTITLE DELETE FILE

 Functional Description:
        Deletes the specified file.

 Calling Sequence:
        delete_file();

 Global inputs:
    instr_desc  = Input string descriptor.  File to delete.
---------------------------------------------------------------------------*/
void delete_file()
{
   int status;

   status = lib$delete_file( &instr_desc );
   if (status & STS$M_SUCCESS)
      fmtoutstr( 1, &SUCCESS );
   else
   {
     edx_signal( 1, &status );          /* Process error deleting file */
     fmtoutstr( 1, &FAILURE );
   }
}
/*---------------------------------------------------------------------------

        .SUBTITLE TRA_EBC_ASC

 Functional Description:
        Translates EBCDIC to ASCII

 Calling Sequence:
        tra_ebc_asc();

 Global inputs:
    instr_desc  = Input string descriptor.  EBCDIC string.

 Outputs:
    outstr_desc = ASCII translation of input string (by descriptor).

 Outline:
    1.  LIB$TRA_EBC_ASC is called to obtain the translation string

;--*/
void tra_ebc_asc()
{
   int status;
   int sig_array[3];
   char buffer[MAXLEN];

   struct dsc$descriptor_s buffer_desc = { instr_desc.dsc$w_length, DSC$K_DTYPE_T, DSC$K_CLASS_S, buffer };

   status = lib$tra_ebc_asc( &instr_desc, &buffer_desc );
   if (status & STS$M_SUCCESS)
   {
      sig_array[0] = SUCCESS;
      sig_array[1] = instr_desc.dsc$w_length;
      sig_array[2] = buffer;
      fmtoutstr( 3, &sig_array);                /* Copy translation to output */
   }
   else
   {
     edx_signal( 1, &status );
     fmtoutstr( 1, &FAILURE );
   }
}

/*-----------------------------------------------------------------------------

        .SUBTITLE TRA_ASC_EBC

 Functional Description:
        Translates ASCII to EBCDIC

 Calling Sequence:
        tra_asc_ebc();

 Global inputs:
    instr_desc  = Input string descriptor.  ASCII string.

 Outputs:
    outstr_desc = EBCDIC translation of input string (by descriptor).

 Outline:
        1.  LIB$TRA_ASC_EBC is called to obtain the translation string

;--*/
void tra_asc_ebc()
{
   int status;
   int sig_array[3];
   char buffer[MAXLEN];

   struct dsc$descriptor_s buffer_desc = { instr_desc.dsc$w_length, DSC$K_DTYPE_T, DSC$K_CLASS_S, buffer };

   status = lib$tra_asc_ebc( &instr_desc, &buffer_desc );
   if (status & STS$M_SUCCESS)
   {
      sig_array[0] = SUCCESS;
      sig_array[1] = instr_desc.dsc$w_length;
      sig_array[2] = buffer;
      fmtoutstr( 3, &sig_array );       /* Copy translation to output */
   }
   else
   {
     edx_signal( 1, &status );
     fmtoutstr( 1, &FAILURE );
   }
}

/*=============================================================================

        .SUBTITLE LIBRARIAN
 Functional Description:
        Dispatches to the appripriate library routine based on the
        low word code value n.

  ^x0007000n - LIBRARY          (458752)
           n = 1.  Initialize, open for read, lookup_key
           n = 2.  Return next line of text from module
           n = 3.  Close text library
           n = 4.  Initialize, open for write, lookup_key
           n = 5.  Write next line of text to module
           n = 6.  Write end-of-module record
*/
#define LBRIOR  1   /* Initialize, Open for read, Lookup_key */
#define LBRIOW  2   /* Initialize, Open for write, Lookup_key */
#define LBRCLO  3   /* Close text library */
#define LBRRNX  4   /* Read next line from module */
#define LBRWNX  5   /* Write next line to module */
#define LBRINS  6   /* Insert (/replace) new module into library */
static int lbr_index;                   /* Library Control Index */
static int lbr_rfa[2];                  /* (Quadword) Current Library Module.  0 if none. */

void librarian()
{
   int incodel;

/* Entry code is cased for reentry point */
   incodel = (*incode_ptr & 0x0000FFFF);
   switch (incodel)                     /* Case entry point to jump to */
   {
      case LBRIOR: lbr_init(LBR$C_READ);        break;  /* 1 = Initialize, Open for read, Lookup_key */
      case LBRIOW: lbr_init(LBR$C_UPDATE);      break;  /* 2 = Initialize, Open for write, Lookup_key */
      case LBRCLO: lbr_close();                 break;  /* 3 = Close text library */
      case LBRRNX: lbr_readnext();              break;  /* 4 = Read next line from module */
      case LBRWNX: lbr_writenext();             break;  /* 5 = Write next line to module */
      case LBRINS: lbr_replace();               break;  /* 6 = Insert (/replace) new module into library */
      default:  edx_signal(1, byref(EDX__UNKNCODE) ); return;
   }
}

/*----------------------------------------------------------------------------

  LBR_INIT

 Functional Description:
        Initialize librarian, open text library for read or write access
        as specified in argument, locate module.

 Returns:
        If error parsing INSTR, signal parse erorr and return 0 in OUTSTR status.
        If error calling LBR$INI_CONTROL, signal error and return 0 in OUTSTR status.
        If error calling LBR$OPEN, signal error and return 0 in OUTSTR status.
        If error calling LBR$LOOKUP_KEY:
            If error was LBR$KEYNOTFND, return 2 in OUTSTR status.
                This may be OK if inserting a new module.
                This may not be OK if we're trying to read a module.
            Any other error:
                signal error and return 0 in OUTSTR status.
        If no errors:
            LBR_RFA = RFA of module found by LBR$LOOKUP_KEY
            Return 1 in OUTSTR status.

 Calling Sequence:
        lbr_init(access);

 Inputs:
        access = LBR$C_READ or LBR$C_UPDATE depending upon whether this
                 is open for read access or open for write access.
                 (by value.  Passed to LBR$INI_CONTROL as "func" parameter.)

        INSTR = Contains text library filename followed by a space character
                followed by the module name to extract
                INSTR = "&lt;text library filename&gt; &lt;module to extract&gt;"

 Outline:
        1.  Initialize librarian - LBR$INI_CONTROL
        2.  Open text library
            a.  Parse off filename from INSTR
            b.  Call LBR$OPEN
        3.  Locate module within text library
            a.  Parse off module name
            b.  Call LBR$LOOKUP_KEY
        4.  Call LBR$SET_LOCATE to set locate mode

 Note:
        The LBR$_... symbols are defined in the SYS$LIBRARY:LBRSHR.EXE
        shareable image.  The linker should resolve these symbols
        automatically, as part of its search through the images in the
        system shareable image library SYS$LIBRARY:IMAGELIB.OLB.

---------------------------------------------------------------------------*/
void lbr_init(int access)
{
   struct dsc$descriptor_s filnam_desc = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 };
   struct dsc$descriptor_s modnam_desc = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 };
   int status;
   int sig_array[5];
   char *p, *end;

/* Parse INSTR for filename */
   end = instr_desc.dsc$a_pointer + instr_desc.dsc$w_length;
   filnam_desc.dsc$a_pointer = instr_desc.dsc$a_pointer;                /* Starting address of "filename" */
   for ( p = filnam_desc.dsc$a_pointer; p &lt; end && *p != 0x20; ++p );   /* Find first SPACE in string - End of "filename" */
   filnam_desc.dsc$w_length  = p - filnam_desc.dsc$a_pointer;           /* Length of "filename" */

/* Parse INSTR for module_name */
   modnam_desc.dsc$a_pointer = instr_desc.dsc$a_pointer + filnam_desc.dsc$w_length + 1;         /* Starting address of "modulename" */
   modnam_desc.dsc$w_length  = end - modnam_desc.dsc$a_pointer;                                 /* Length of rest of string */
   if (modnam_desc.dsc$w_length &lt;= 0)
   {
      edx_signal( 1, byref(EDX__NOMODNAM) );            /* No input module name. */
      fmtoutstr( 1, &FAILURE );
      return;
   }

/* Initialize library control structure */
   status = lbr$ini_control( &lbr_index, &access, &LBR$C_TYP_TXT );
   if ( !(status & STS$M_SUCCESS) )
   {
      edx_signal( 1, &status );
      fmtoutstr( 1, &FAILURE );
      return;
   }

/* Open library file */
   status = lbr$open( &lbr_index, &filnam_desc );
   if ( !(status & STS$M_SUCCESS) )
   {  /* Error opening specified text library.  Signal error open, then signal error */
      sig_array[0] = EDX__ERROPEN;              /* Error opening file */
      sig_array[1] = 1;                         /* one FAO argument */
      sig_array[2] = &filnam_desc;              /* Text library file name */
      sig_array[3] = status;
      edx_signal( 4, &sig_array );
      fmtoutstr( 1, &FAILURE );
      return;
   }

/* Set record access to locate mode */
   status = lbr$set_locate( &lbr_index );
   if ( !(status & STS$M_SUCCESS) )
   {  /* unusual error during lbr$set_locate.  Signal error, close library, return 0 */
      edx_signal( 1, &status );
      status = lbr$close( &lbr_index );
      if ( !(status & STS$M_SUCCESS) ) edx_signal( 1, &status );
      fmtoutstr( 1, &FAILURE );
      return;
   }

/* Look for module in library */
   status = lbr$lookup_key( &lbr_index, &modnam_desc, &lbr_rfa );
   if ( status == LBR$_KEYNOTFND )
   {
      lbr_rfa[0]=0; lbr_rfa[1]=0;               /* no old module in library */
      if (access == LBR$C_READ)
      {
         sig_array[0] = EDX__MODNOTFND;         /* text library module not found */
         sig_array[1] = 2;                      /* two FAO arguments */
         sig_array[2] = &modnam_desc;           /* Module name */
         sig_array[3] = &filnam_desc;           /* Text library file name */
         sig_array[4] = status;
         edx_signal( 5, &sig_array );
         status = lbr$close( &lbr_index );
         if ( !(status & STS$M_SUCCESS) ) edx_signal( 1, &status );
      }
      fmtoutstr( 1, &2 );                       /* Key not found status */
   }
   else if ( !(status & STS$M_SUCCESS) )
   {
      edx_signal( 1, &status );
      status = lbr$close( &lbr_index );
      if ( !(status & STS$M_SUCCESS) ) edx_signal( 1, &status );
      fmtoutstr( 1, &FAILURE );                 /* error other than 'Key Not Found' */
   }
   else
      fmtoutstr( 1, &SUCCESS );                 /* found key success */
}

/*---------------------------------------------------------------------------

 LBR_CLOSE

 Functional Description:
        Closes text library freeing up internal storage used by librarian.

 Calling Sequence:
        lbr_close();

 Inputs:
        LBR_INDEX - set by LBR_INIT

 Outputs:
        OUTSTR - status.

---------------------------------------------------------------------------*/
void lbr_close()
{
   int status;
   status = lbr$close(&lbr_index);              /* close library */
   if (status & STS$M_SUCCESS)
      fmtoutstr( 1, &SUCCESS );                 /* Normal success */
   else
   {
      edx_signal( 1, &status );
      fmtoutstr( 1, &FAILURE );
   }
}

/*---------------------------------------------------------------------------

 LBR_READNEXT

 Functional Description:
        Returns next line from text module.  Assumed LBR_INIREAD already
        called to initialize library.

 Calling Sequence:
        lbr_readnext();

 Inputs:
        LBR_INDEX - set by LBR_INIREAD

 Outputs:
        OUTSTR - Next line from module within text library.


---------------------------------------------------------------------------*/
void lbr_readnext()
{
   int sig_array[3];
   struct dsc$descriptor_s outbuf_desc = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 };

   sig_array[0] = lbr$get_record( &lbr_index, 0, &outbuf_desc );
   sig_array[1] = outbuf_desc.dsc$w_length;
   sig_array[2] = outbuf_desc.dsc$a_pointer;
   fmtoutstr( 3, sig_array );
}

/*---------------------------------------------------------------------------

 LBR_WRITENEXT

 Functional Description:
        Write next line to text module.  Assumed LBR_INIWRITE already
        called to initialize library.

 Calling Sequence:
        lbr_writenext();

 Inputs:
        INSTR - Next line to write to module
        LBR_INDEX - set by LBR_INIWRITE

---------------------------------------------------------------------------*/
void lbr_writenext()
{
   int status;

   /* Write next record to module */
   status = lbr$put_record( &lbr_index, &instr_desc, &lbr_rfa );
   if ( status & STS$M_SUCCESS )
      fmtoutstr( 1, &SUCCESS );
   else
   {
      edx_signal( 1, &status );
      fmtoutstr( 1, &FAILURE );
   }

}

/*---------------------------------------------------------------------------

 LBR_REPLACE

 Functional Description:
        Insert (/replace) text module into library.

 Usage:
        The sequence for inserting a text module is as follows:
                CALL LBR_INIT( LBR$C_UPDATE )
                IF (STATUS = 2) THEN
                  ASK USER FOR PERMISSION TO REPLACE EXISTING MODULE;
                LOOP
                  CALL LBR_WRITENEXT            !Pass lines of text to library creating module
                ENDLOOP
                CALL LBR_REPLACE                !Insert new module into library index
                                                !(If previous module existed, it is deleted at this time)

 Calling Sequence:
        lbr_replace();

 Inputs:
        LBR_INDEX - Library Control Index.  Set by LBR_INIT by LBR$INI_CONTROL
        LBR_RFA   - Current Library Module.  Set by LBR_INIT by LBR$LOOKUP_KEY
        INSTR     - Name of text module to insert.

 Outline:
        The sequence is similar to the DCL command $ LIBRARY/TEXT/INSERT
        See INPUTTXT.LIS in facility LIBRAR

---------------------------------------------------------------------------*/
void lbr_replace()
{
   int status;
   int oldrfa[2];

   /* WRITE END-OF-MODULE RECORD */
   status = lbr$put_end( &lbr_index );          /* write end-of-module record */
   if ( !(status & STS$M_SUCCESS) )
   {
      edx_signal( 1, &status );
      lbr_close();
      return;
   }

   /* SEE IF MODULE NAME ALREADY EXISTS */
   status = lbr$lookup_key( &lbr_index, &instr_desc, &oldrfa );
   if ( status == LBR$_KEYNOTFND )
   {
      oldrfa[0]=0; oldrfa[1]=0;                 /* no old module in library */
   }
   else if ( !(status & STS$M_SUCCESS) )
   {
      edx_signal( 1, &status );
      status = lbr$close( &lbr_index );
      if ( !(status & STS$M_SUCCESS) ) edx_signal( 1, &status );
      fmtoutstr( 1, &FAILURE );                 /* Key not found status */
      return;
   }

   /* INSERT NEW MODULE */
   status = lbr$replace_key( &lbr_index,        /* Library control index (read)*/
                             &instr_desc,       /* key-name (read) */
                             &oldrfa,           /* old-module (read) [note error in VMS 4.4 documentation calls this "write only"] */
                             &lbr_rfa );        /* new module (read) */
   if ( !(status & STS$M_SUCCESS) )             /* on error signal, attempt close, and format bad return status */
   {
      edx_signal( 1, &status );
      status = lbr$close( &lbr_index );
      if (status & STS$M_SUCCESS)
      {
         lbr_rfa[0]=0; lbr_rfa[1]=0;            /* no current library module */
      }
      else
         edx_signal( 1, &status );
      fmtoutstr( 1, &FAILURE );
   }

   if ( oldrfa[0] != 0 || oldrfa[1] != 0 )      /* If old module then delete old module */
   {
      status = lbr$delete_data( &lbr_index, &oldrfa );  /* Delete old module */
      if ( !(status & STS$M_SUCCESS) ) edx_signal( 1, &status );
   }
   lbr_close();

}
