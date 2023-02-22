

ActionScript is a simple scripting language, very much like DOS' Batch commands.

This file will serve as a quick reference for the commands available.

Reboot
Shutdown
PowerCycle
ResetMenuCache
TrayOpen
TrayClose
Format <Drive>
AllowFormat <1|0>
Attr <FileOrDir> ro|rw
Chmod 777 <FileOrDir>   (This command is compatible with FlashFXP's "Attributes" action)
LaunchDVD
EEPROMBackup [Filename] (Backs up EEPROM contents, encrypted, to a file, file defaults to "eeprom.bin" in current directory)
EEPROMRestore [Filename] (Restores EEPROM contents, encrypted, from a file, file defaults to "eeprom.bin" in current directory)
MkDir <PATH>
RmDir <PATH> <CASCADE>
CD <PATH> (GUI only, sets default directory, which starts as MXM's path)
Copy <Path/File/Wildcard> <Path/File>
Rename <FileOrPath> <FileOrPath>
Delete <File>
Goto <Label>
SystemMenu <Activate|Hide|Unhide|ActivateFull>
MessageBox <message> (Displays a message box AFTER script is done executing)
Set <VarName> <Value>  
Quit
:<Label>
ADD <VarName> <Value> (Numerically adds value to variable's value)
SUB <VarName> <Value>
DIV <VarName> <Value>
MULT <VarName> <Value>
IF <arg1> <op> <arg2> goto <label>   (Evaluates by comparing strings)
     <op>:
      =
      >
      <
      >=
      <=
      != or <>
      ##   (True if LValue contains RValue)
      #^   (Same as ##, but case insensitive)
      !##   (True if LValue doesn't contain RValue)
      !#^   (Same as !##, but case insensitive)
IF# (Used to evaluate numerically)
SETFANSPEED <Manual|Default> <speed>
SETFUNC <VarName> <Function> <Function Arguments>
 Functions:
 - XBETITLE <FileName>
 - XBEID <FileName>
 - FILESIZE <FileOrPath>
 - FILEEXIST <FileOrPath>
 - PATHEMPTY <Path>
 - LEFT <Count> <Text>
 - RIGHT <Count> <Text>
 - MID <Count> <Length> <Text>
 - SPACE <Count>
 - REPEAT <Count> <Text>
 - LENGTH <Text>
 - TRIM <Text>
 - TRIMLEFT <Text>
 - TRIMRIGHT <Text>
 - LOWER <Text>
 - UPPER <Text>
 - CONDITION or COND <conditional>    (Sets value to 1 or 0, depending on state of skin conditional)
 - !CONDITION of !COND <conditional>
EXEC <xbe> [<args>]
BeginDraw [UseCurrent]
EndDraw
Input
CallFile <file> <Args...>
CallScript <name> <Args...>
Box <X> <Y> <Width> <Height> [<Color>] [<BorderColor>]
Text <X> <YTOP> <CENTER|LEFT|RIGHT> <Text> [<Color>]
ALLOWFORMAT <1|0>
MOUNT <Drive:> [<DevicePartitionPath>]
DATE <Day> <Month> <Year>
TIME <00:00>
DELAY <Delay Sec>
DELAYMS <Delay ms>



==============Variables===============


 Use alphanumeric characters, prefix with underscore for "persistant" variables, that is, variables prefixed with an underscore ("_") will retain
 their value after the script is done running, and may be examined by other scripts. Variables are used in arguments as follows:

  SET SomeVar "This is a test"
  MessageBox %SomeVar%

  Will output a message box that says "This is a test"

 Likewise, you can access system strings, just as in layouts, but use the string name bounded by "$" characters:

  SET SomeVar "The Time Is: $time$"
  MessageBox %SomeVar%

  Will output a message box that says something like "The Time Is 11:39pm"

  $eol$ is a special variable that inserts the CR/LF sequence into a string.... useful for message box text.


Important note on arguments for actions... if it's quoted, then the BACKSLASH is an escape character, meaning you'll need to 
"double up" to see a single BACKSLASH in any quoted arguments. This is not an issue on arguments that are NOT quoted. You only
need to quote arguments that contain spaces.

You can create an Install Disc, ala "Slayer's" with something like the following:
<Action>
FORMAT C:
FORMAT E:
FORMAT F:
FORMAT G:
COPY D:\CDrive C:\
COPY D:\EDrive E:\
COPY D:\FDrive F:\
COPY D:\GDrive G:\
</Action>

=================INPUT VARIABLES=====================
Current "Input Variables" supported:

_GP_A
_GP_B
_GP_X
_GP_Y
_GP_BLACK
_GP_WHITE
_GP_START
_GP_BACK
_GP_D_UP
_GP_D_DN
_GP_D_RT
_GP_D_LF
_GP_TRG_RT
_GP_TRG_LT
_GP_THUMB_LF
_GP_THUMB_RT

The list will grow....



Example Script using BeginDraw:


<Action>
BeginDraw UseCurrent
MessageBox "This is a message$eol$Second Line"
EndDraw
Input
If %_GP_A% == "1" GOTO APRESSED
If %_GP_B% == "1" GOTO BPRESSED
If %_GP_X% == "1" GOTO XPRESSED
If %_GP_Y% == "1" GOTO YPRESSED
QUIT
:APRESSED
MessageBox "A Was Pressed!"
QUIT
:BPRESSED
MessageBox "B Was Pressed!"
QUIT
:XPRESSED
MessageBox "X Was Pressed!"
QUIT
:YPRESSED
MessageBox "Y Was Pressed!"
</Action>




=================COLOR NAMES=====================
WHITE
BLACK
GRAY
BLUE
RED
GREEN
YELLOW
CYAN
VIOLET
ORANGE
BROWN

You may also add "LIGHT" or "DARK" as a prefix, names are not
case sensitive:

LightGreen is the same as LIGHTGREEN

It also accepts hexadecimal ARGB "Quads" such as 0xFF112233
See the Skinning Overview for details.



==============INTERNAL SCRIPTS===================
This is a copy of the Internal XML node used to 
hold Internal ActionScripts

<Scripts>
<ActionScript Name="_DisplayMessageBox">
BeginDraw UseCurrent
MessageBox %1%
EndDraw
Input
</ActionScript>
<ActionScript Name="_AutoExec">
IF $MXMVersion$ !#^ "WIP" GOTO DONE
CallScript _DisplayMessageBox "This is a WIP Release$eol$Do not distribute"
:DONE
</ActionScript>
</Scripts>


===================Special ActionScript symbols==================

Special ActionScript symbols, $eol$, $lt$ and $gt$

	