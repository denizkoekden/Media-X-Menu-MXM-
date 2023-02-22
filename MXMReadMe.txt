
Media X Menu Version 0.9n.6 Beta

Tutorials Page at: http://www.writersgallery.com/MXM/TUT


NOTE: Do not use prior versions (you've been warned!)


Media X Menu is a menu system to launch applications from a DVD collection or from 
a hard drive. It is intended to be very configurable and allow new "styles" to be added 
in, both with simple visual changes (skins), or adding 3D effects (later) to the presentation
of the menus by re-writing the software.

It was intended to supercede Complex Menu X, which has some bugs and is lacking many 
desired features that MXM now has.

It is now in "Beta" status. It should be stable to use on DVD-R discs, but who knows?
As usual, use at your own risk. 


I am releasing this source code for others to use for both their own projects, and 
hopefully, to enhance this project. The current features are as follows:

There *ARE* still some bugs with this thing.... but I've tried to do as much testing as 
possible without spending three weeks on it. I believe most of it relates to the 
handling of the soundtracks at the moment, but tracking down everything is time consuming
and at the moment, it seems to work 99.9% of the time. My suggestion would be to not use 
"random" soundtrack play, and it will be turned of by default.

* Impliments the "Simple" theme, allowing a title screen, menu screen, loading screen 
  and saver screen, each with configurable elements (custom backgrounds, custom bouncing 
  logo for the screen saver)
* Simple theme uses Skins based in subdirectories, and can use randomly selected skins 
  on every run of the program.
* Launches applications from the hard drive and the CD/DVD drive.
* Screen saver! 
* Can play one or more music tracks, with multiple options, such as-
  - Disc-specific tracks (played from subdirectory, for example)
  - Can play from the XBox soundtracks the user has on his hard drive
  - Can play random or in order
* Supports BMP, PNG, TGA, JPG image formats as well as WMV video files!
* All settings are done via XML file support, a format familiar to many users.
* Supports 640x480 at the moment. Sorry, just ran out of time to get the multi-res
  support in.

==================================================
===   IR Remote Support                        ===
==================================================

Thanks to d7o3g4q and RUNTiME for releasing their source code as without this Remote support wouldn't have been possible.

Here is the latest Build with full IR Remote Support.

The COntrols are as Follows:
============================

IR Remote up/down = scroll trhough the menu list
IR Remote Select  = Select's the Game / Emulator
IR Remote Info    = Media X Menu Version Info Displayed
IR Remote Back    = Return To Media X Menu Title Screen
IR Remote 0       = Reboot XBOX back to Dashboard

Any button on the IR Remote will come out of the screensaver and skip past menu's like the 
Controller does.

IR Support By SpOoK with thanks to d7o3g4q and RUNTiME for their IR Remote Code.

Note from BenJeremy: I don't have the remote, but the code looks good and SpOok has tested this
with most of the releases. 

==================================================
===   A Note about XML                         ===
==================================================

For those not familiar with XML, it's basically a method of tagging information in a text file.

Configuration XML files in MXM, for example take on the following structure:

<Config>
 <Main>
  <DiscTitle>My Collection</DiscTitle>
  <DiscSubtitle>MAME</DiscSubtitle>
  <Theme>1</Theme>
 </Main>
</Config>


The entire config is surrounded by the <Config> tags, and each section has their own group.

Generic skins will get much more sophisticated, due to the nature of what needs to be done...
When these are implimented, there will be a very detailed description of how these XML
files must be built.

In the meantime, all INI files can be converted into MXM XML files using the INItoXML 
converter for Windows in this binary release.



==================================================
===   Auto-Config Items                        ===
==================================================

To make things a bit more bulletproof, MXM now supports "Auto-config" I would
encourage those who make software releases for the Xbox to include a game/logo
screenshot and an MXM_Entry.xml file as part of the release.

Auto-config loading sequence:
1. First MXM loads all the entries in the MXM.xml file
2. MXM then searches the IMMEDIATE subdirectories for MXM_Entry.xml
   files. If found it will attempt to load those values... missing values
   or non-existant files will cause MXM to fall back to the following steps.
3. If an executable has not been assigned by the MXM_Entry.xml file, MXM will 
   look for "default.xbe", failing to find default.xbe, it will search for the 
   FIRST *.xbe file it finds in the directory and use that.
4. If an executable has been found, but no title has been assigned, it will
   use the title stored as part of the XBE's built-in certificate.
5. Lastly, if there is no game/logo screenshot assigned at this point, the system
   will look for the first MXM_SS.* file that is a JPG, BMP, TGA, or PNG and use
   that file.

So the easiest thing to do, for each game, is to simply through a screenshot in the 
directory named "MXM_SS.jpg" (or whatever the extension is for the chosen format).

Next easiest thing is the above screenshot file, with an MXM_Entry.xml file including 
the following:

----------
<Config>
<Item>
<Title>Mame Vol#3</Title>
<Description>CAPCOM collection!</Description>
</Item>
</Config>

----------

The <Item> node may also include the following:

<Media>[image filename]</Media>
  The image file, relative to the location of this entry ini file.
<Exe>[xbe filename]</Exe>
  The filename only, of the XBE to use in this directory.
  Ex: "Exe=XMarbles.xbe"




===================================================================
FAQ:
===================================================================
Q: I've got an Enigmah Beta, why doesn't this run?
A: I don't have a patch tool for Enigmah users. Sorry.... if you 
   have such a tool, please get a hold of me at xbox-scene or 
   xboxhacker.net.

Q: I don't like the title screen. Can you get rid of it?
A: Well, it's simple enough to give the title screen delay a value of '0'
   to skip by it.

Q: I don't like the screen saver feature, how do I get rid of that?
A: Same thing, a delay time of zero will de-activate the saver.

Q: Where can I find skins?
A: I'll try and release a few, but at the moment, nobody is hosting 
   skins specifically for my menu system; HOWEVER, since the 'default'
   layout for simple happens to match up to Complex MenuX, you can use
   skins created for that system with this menu (of course, MXM is far
   more flexible in positioning screens and items)

Q: How about a tutorial on using this?
A: I'd love to put one together for it, but to be honest, I'm probably 
   the wrong person to do the job (as the developer, I'd have a hard 
   time making a tutorial from a nontechnical perspective). If anybody out 
   there feels up to the task, please do and send it to our good 
   friends at xbox-scene.

Q: I can't get this to boot.
A: Well, I need a copy of your INI file, at the very least, in order to 
   determine what the problem is. Try checking your image files and make
   sure they are there and are correctly formatted. Get a hold of me at 
   xbox-scene if all else fails.

Q: I don't like the Sky-and-logo splash screen. How do I get rid of it.
A: You don't.
   1) Internal images: Does not require ANY external resources to load. 
      External Resources=BAD.

   2) It's Initialization. If MXM crashes, I have a very good idea WHERE 
      based on the progress with relation to the splash screen (before during 
      or after)

   3) Well, it's initialization, you don't want to load up anything externally 
      (there's that first point again). You can't grab information from a config 
      file that hasn't been loaded yet.

   4) It's an ID logo. Call me vain, but it's been months of hard work to put 
      this together. At least I haven't put my face or something there.

   5) MXM is open source. If somebody wants to, they can re-write the code 
      themselves for their personal version. They can also support such a 
      version when people screw up their config files or something and can't
      boot - because it will be harder to do.

   6) Loading times. Part of the reason it was added was to let people know MXM 
      was doing something while it spent the time loading files and looking for
      executables. (though that load time has been reduced a bit)

   Look, I know the sky-and-logo clash sometimes with people's skins. That's part
   of the reason I fade it to black before displaying the title screen or menu screen.

   The new version will be enhanced by adding a variety of "Init Screens", including a 
   Matrix-style tumbling symbol thing. I am also considering adding a plasma-lightning 
   animation as well. You will be able to choose with a simple patch to the XBE file. The 
   splash will ALWAYS be there in my versions, in some format.



===================================================================
HISTORY:
===================================================================

Alpha 0.2
  Minor bug fix release
* Fixed problem identifying the correct partitions. (Thanks ITM of xboxhacker.net for the fix)
* Added speed enhancement, again, thanks ITM for this code.
* Moved soundtrack to it's own thread to prevent pausing in screen saver
* "About" box added  when pressing "White button" in the menu screen
* Added the  PersistDisplay() call, but didn't notice a difference. No loading screen joy.
* Added "DrawDescription" to Menu section to allow you the ability to turn off the description.
* Keyboard support - Cursor Up/Down, ENTER to select. Keys skip title and saver, F1 brings up 
  "about box" and PrintScreen key will reboot to the dashboard.

Alpha 0.3
  Minor bug fix release
* Fixed problem identifying the correct partition for F:. AGAIN.
  Bah! The generic code didn't want to work, even after I identified
  a problem with the code. Fixed it with a specific case for F:

Alpha 0.4
* More bug fixes! Didn't realize that C:, E:, and F: were unmounted, causing 
  problems when the image files didn't "live" in the directory below the 
  menu system and get called as "D:\***" This problem is now fixed.
* Changed option "Theme" in section [Main] to "Style" This will represent the
  type of menu displayed (simple, or in the future, 3-D styles)
* Smooth scrolling menu items and descriptions.
* Menus and description placement are fully configurable. Defaults to MenuX style,
  to be compatible with existing MenuX skins. Now menus, screenshots, and the 
  description can be placed anywhere on the screen and in just about any size.
  You can also select the maximum number of options displayed at any given time
  (that is, how many item rows are displayed in the menu item box)

Alpha 0.5
* Phew. Bug fix. Sorry about that, people, but there was a big bug causing 
  instability in the system. It's very difficult to debug an app wthout a 
  debug console. Many problems solved now.
* SKINS! SKINS! SKINS! SKINS! This is the first release supporting 
  self-contained skins! "DefaultSkin" is included with the release,
  basically the same images and sounds as in the previous releases, now
  in a handly subdirectory with their own ini file.
 
Beta 0.6
* Cleaned up a couple of cosmetic bugs.
* Fixed a problem with the skin soundtracks
* Implimented a few more skin attributes. Not all are there yet.
* Changed the screensaver slightly.
* Release will be a bit cleaner than before, with the ini files 
  cleaned up.

Beta 0.7
* Fixed a bug that prevented skins from working in 0.5 and 0.6
  I now will test each release with a XDFS formatted CD-RW (problems and
  time make testing on DVD-RW or DVD-R prohibitive at the moment)

Beta 0.8
* Fixed issue with descriptions that are missing.
* Cleaned up message box popping up when a screenshot is missing.

Beta 0.9
* AutoConfig!! See the section on AutoConfig for complete details.
* Sped up menuing by offloading screenshots to a seperate thread.

Beta 0.9b
* Added IR support, courtesy of SpOoK from the  xboxhacker.net forums
  (See the section on the IR support)
* Fixed 'overzealous' keypress issue.
* Changed dashboard sequence for the controller from BACK+START to RIGHT STICK+LEFT STICK
  It remains "PrntScr" for the keyboard.

Beta 0.9c
* Added the ability to navigate using the thumbsticks
* Fixed the PersistDisplay() issue. Loading screens now display.
* WMV is now a supported format! The videos will loop, and sound is disabled
  in this release by default. I'll add parameters to change those (sound does
  work) shortly. WMV files can be used ANYWHERE an image file is used.
  Thanks go out to Syn3rgy @ the xboxhacker.net forums for knocking down the
  remaining barriers on this feature.
* Moving away from ini files. XML will be the new standard (faster loading)
  To make things easier, I've included a converter, to modify existing files to 
  the new format.
* Disable for auto-config - Set Main->AllowAutoConfig to No (or Off/false)
* Main->MoveDelay - Time between pressing down/up and the menu changing selections in Repeat mode.
* Main->RepeatDelay - Time between holding down/up and the program repeating the move.
* Main->LoadSkin parameter to select a specific skin (Based on directory name)
* Main->ExitApp allows you to specify a dashboard other than the 'default' one, such 
  as "C:\evoxdash.xbe" when using the dashboard sequence. This solves a problem
  when using MXM as your default dashboard application.
* Well, a lot of work, but I've sped things up a bit (well, not really sped up, as
  much as shuffled things around) and added a nice fade in to a moving sky and MXM logo
  just before hitting the title screen.
* Smooth transitions now occur between menu states
* the slight "stutter" from the music when MXM starts up is now fixed.


Beta 0.9d
* Fixed a problem that prevented Auto-config from finding the correct *.xbe file, even if
  it was specified in the MXM_Entry.xml file. It will work correctly now.
* Added WMV as an Auto-config media type (It will use a file named MXM_SS.wmv in the game 
  directory.)

Beta 0.9e
* Fixed glitchy graphics issues. This included messed up looking menus and the odd flash before 
  the title screen faded in.
* Changed the "intro" screen to fade out to black before the title screen. The textures are no longer
  loaded externally.
* Fixed (hopefully all) the issues with the music manager. Random and global should work with skin
  soundtracks.
* Dpad, stick and IR direction "repeat delay" issues are now resolved. They should act the same.
* Added "VerticalSelection" to "Menu" options, defaults to TRUE. When set to FALSE, will allow the 
  controller input Left to Right to select menu items.
* I ==THINK== I have the empty description bug fixed. It has been tested, and should work. The details 
  are still a mystery to me, though... but my fix has done the trick.
* A little change in the way autoconfig works... if you define entries in the MXM.xml file, then
  AllowAutoConfig defaults to FALSE, if there are no entries in the MXM.xml file, it defaults to
  TRUE. Of course, you can specify your own value for this as well.

Beta 0.9f
* Moved "Font16.xpr" internal to the xbe, so no media directory is required anymore.
* Added the ability to put passcodes in the game entries. This will allow those with children 
  to prevent easy access to games without having to enable parental controls or prevent unauthorized
  access to programs. (I added this so my 10 year old son would not have easy access to BMXXX)
* Changed IR remote "boot to dash" function to "Title" (Was "0" key)
* Added support for MXM and game help screens. There are 3 built-in help screens for MXM. See "Entry"
  and "Help" configuration for more details on these options.
* Help for games may also be auto-config'd like the screenshots, using MXM_GHxx.* where xx is a value
  from 01-99 (must start with 01 and continue contiguously) and '*' is a valid image extension.
* A TitleDelay of 0 prevents the display of the title screen.
* Added a TitleImage.xbx icon to be 'friendly' with NexGen. It will install the icon on startup, if it
  doesn't already exist (internally generated, no extra files needed).
* Cleaned up the launch code a bit. Now it checks to make sure a title exists before launching it;
  if the xbe does not exist, it puts up a detailed message and kicks back to the menu screen.
* Added ability of AutoConfig items to pull title and description from the Game's ID. A few titles
  have regular descriptions (in many cases, this is the part of the title following the colon) and
  for those without, the publisher's name can be used in their place.
* Odd, for some reason, the system was not initializing all of the drives (T:, U:).
* MXM should run without any MXM.xml file or support files. If it doesn't find entries, it 
  will display the likely entries it finds for a dashboard.
* A new switch "AllowVideoSound" allows you to pause the music when videos are shown as
  screenshots, so the audio may play (good option to use on DVDs, too, due to speed/disc access
  issues)

Beta 0.9g
* Reorganizing MusicManager to stomp all known issues with song selection and local soundtrack.
* Added two additional "intro" screens that can be configured with patching
  0 = Matrix Effect intro screen
  1 = Large Logo with sky
  2 = Small Logo with sky
* Added Matrix effect to screen saver; Saver will SOON allow combination of several elements:
  - Background can be black or shifting colors
  - Matrix Effect "falling symbols"
  - Bouncing MXM logo
* Added a system menu accessed with "Back" button
* Fixed memory/resource leak with fading screen logic
* Matrix Symbolic font was added to "built-in" data - Font designed by me (BJ), includes 12
  symbols and letters A,D,E,I,M,N,U,X	
* MenuX Compatibility BROKEN - DO NOT USE THIS ONE - SORRY.

Beta 0.9h
* MenuX Compatibility - MXM's default.xbe can be dropped in as a direct replacement
  for the MenuX app... it will use the XDI file to configure the games, and also use the
  PNGs and sounds from the MenuX media directory as a 'skin', unless another skin is loaded
  (If 'real' MXM skins are available, the MenuX 'skin' will be randomly selected with the 
  other choices).
  Was broken on 0.9g - That's what I get for not testing completely!! I am very sorry, while
  I could rattle off a slew of excuses, it doesn't change the fact that I screwed up.

  Expect the goodies in the next version. None of the future features were ready for prime time
  at the time of this bug fix.

Beta 0.9i
* Setting TitleDelay to "-1" will leave the title screen on until the user presses a button 
  (Unless <EndAfterSequence> selected in image for layout)
* New skinning technology (See accompanying SkinningOverview.doc for details)
  Key Features:
  - Font cache: Fonts are skin loadable resources. One is built in, and the MenuX fonts are loaded in 
    in that mode.
  - "Old School" MenuX style menublock. While MXM's version is much more configurable, in MenuX mode, we
    make it look EXACTLY like the original, provided correct fonts are present. 
  - Dynamic text fields with numerous effects using modifiers. Uses fonts from cache.
* Added capability to use SubMenus and load from secondary menu files. This is a big one... still
  works the old way, but with true XML loading of the menu information, you can now group items in a variety
  of ways and also you don't need to worry anymore about the <Item_1> indexing.
* I think the dashboard issues are fixed. More detail will be explained in the new User's Guide. It should
  run from any partition, but it does so by searching for itself as a dashboard (this is due to the fact that
  I have no way that I can tell where MXM is run from).
* MXM supports multiple directories and files to be added to the local soundtrack (supported in skin)
* Added power off to System Menu
* Supports TitleImage.xbx (Program Icons) for image types. Using the XBE as an image source now redirects to the 
  appropriate TitleImage.xbx file.
* AutoDir supports "Flatten" and "Recurse" flags. Flatten keeps all entries, regardless of how deep they are in the
  directories, in the same menu level as the AutoDir tag - defaults to FALSE. Recurse, causes the AutoDir search to look 
  into subfolders beyond one level down - defaults to TRUE.
* <RemoveDuplicates> option will eliminate multiple occurances of the same app from popping up. Defaults to TRUE.
* Internal images can be used as resources. These have unique filenames:
  ::SkyTile
  ::Logo
  ::HelpGCScreen
  ::HelpKBScreen
  ::HelpIRScreen
* Ugh. Dashboards menu doens't work too well. Not sure what's going on there. Hopefully it will be fixed by 0.9h
* FTP server is enabled, but it will likely lock up on use. It requires a valid FileZilla.xml file, and I've run out 
  of time to get it sorted out.

Beta 0.9j
* Does not exist. There is no spoon. These are not the versions you are looking for.

Beta 0.9k
* Fixed a nasty parsing bug in the XML parser that was hosing menu XML configs.
  The parser I used failed to consider nested nodes with the same node name, so it 
  would stop parsing the node at the first end tag, instead of doing a reference count
  and incrementing/decrementing the reference count on pairs of tags. Understandable, 
  as the code gets more complex. Hopefully my fix will hold :)
* Fixed problem with FontBundle toolkit created XPR files. The tool used a newer version
  of the XDK tool, resulting in a version conflict with the MXM font routines. MXM now 
  supports both versions of fonts without complaint.
* Soundtrack issues seem to be cleaned up.
* Fixed problem with "Passcode" conditionals
* FTP Server Working! No longer using XBFileZilla, using J.P.Patches' code. Not perfect yet, as
  I've got configuration and such. For now, l/p: xbox/xbox Configuration will come in on the next version.
* Fixed minor issue with MXM style menu block - if you specify "Scale Y"a dn number of items, it didn't use 
  the proper height, and failed to display allthe items, now it does.
* Fixed synchronization issue with gadget timers
* Added new strings:
   MAC
   XboxVersion
   SerialNumber
   VideoStandard
   ShortDriveCSize
   ShortDriveESize
   ShortDriveFSize
   ShortDriveCFree
   ShortDriveEFree
   ShortDriveFFree
* Text elements can be used as horizontal menus. Suggested usage is to include conditional triangles on either
  side, with "CGTitle" as the string source... Tag for Text Gadgets is: <IsMenu Horizontal="true">true</IsMenu>
  "Horizontal" attribute changes the control to left/right instead of up/down.
  All the usual Text attributes apply... so have fun with it.
* Cacheing Menus! Turn off this feature in the MXM.xml with <UseMenuCache>False</UseMenuCache> under the Main node.
  Otherwise, cache_menu.xml will be generated when running from the hard drive, and used by the program
  whenever it is more current than the MXM.xml and/or the menu.xml file.
  It can be reset from the System Menu (Y Button)
* Fixed problem with Soundtracks! Didn't reveal itself until testing for release... The problem would cause
  lockups on the transition between Title and Menu states.

****************************************************************************************************************
****************************************************************************************************************
Big shout out to J.P.Patches for his FTP Server code.

J.P.Patches shouted back to send back a thanks to Myself (BenJeremy) and HSDEMONZ for assisting in the testing 
of the server, way back when...
****************************************************************************************************************
****************************************************************************************************************


Beta 0.9l
* New strings:
  TempSys
  TempCPU
  TempSysF
  TempSysC
  TempCPUF
  TempCPUC
  BIOSMD5
  BIOSName
  CurrentSubMenu
  TrayState (Open/Closed/Init)
  DVDType  (Identifies the type of disc currently inserted: Game, Movie, Unknown, Empty)
  Name (Identifies the nickname)
  CurrentSong
  CurrentSoundtrack
  NumFTPUsers
  DefaultGateway (These new IPs do not have 'dressing' - add with Compound Strings)
  SubnetMask
  DNS1
  DNS2
* Added Preferences node to configuration.
  - <UseCelsius> If True, displays temps in degrees Celsius, otherwise uses Farenheit
  - <TimeFormat> 0=24-hour format, 1=am/pm format
  - <DateFormat> 0=American mm/dd/yy, 1=Euro yy/mm/dd (More to follow)
  - <Nickname>   Adds the nickname of your choice to the head of the nickname cache
  - <AutoLaunchGame> Will run single game discs and load multi-game compilations into the menu (MXM and MenuX)
    Defaults to FALSE
  - <AutoLoadMulti> Loads multidisc DVD compilations into the menu (Like AutoLaunchGame, but won't run single game discs)
    Defaults to TRUE
  - <DeepMultiSearch> When loading discs, this indicates whether the AutoLoad should search beyond the first layer of subdirectories
  - <AutoLaunchMovie> Will run movie discs using the selected movie player
    Defaults to FALSE
  - <DVDPlayer> and <DVDPlayerCmdLine> get called when playing a DVD movie disc that's autolaunched
                 If not specified, it will search all menu items for the DVD Region X v2 player.
* Added "Overrides" - have to be careful here.... overriding music/video interaction could 
  lead to issues!!
   Overrides::PreviewEnableSound True/False
   Overrides::PreviewSuspendMusic True/False
   Overrides::ForceGlobalMusic True/False
   Overrides::ForceLocalMusic True/False
   Overrides::ForceRandomMusic True/False
* Added new conditionals:
   SubMenu   - True when current menu displayed is submenu
   MainMenu  - True when current menu displayed is main menu (topmost)
* Fixed potential issues with multiple FTP users accessing DVD drive.
* Item element <CommandLine> now supported. Untested, but should work.
* On StrongBad's suggestion, added "B" button to act as "Back" when navigating menus.
* Fixed nonworking <ArrowColor> tag in Menu::MenuX gadget.
* Skins now may assign menu and message box colors:
   MessageBox::BoxColor (Defaults to 0x80000000)
   MessageBox::BorderColor (Defaults to 0xffffff00)
   MessageBox::TextColor (Defaults to 0xffffffff)
   MessageBox::ShadowColor (Defaults to 0xff000000)
   SystemMenu::BoxColor (Defaults to 0x80000000)
   SystemMenu::BorderColor (Defaults to 0xffffff00)
   SystemMenu::TextColor (Defaults to 0xff909090)
   SystemMenu::SelectColor (Defaults to 0xffffffff)
   SystemMenu::ValueColor (Defaults to 0xff000090)
   SystemMenu::SelectValueColor (Defaults to 0xffc0c0ff)
* Added check to prevent AutoDir from loading MXM as an item
* "X" button on gamepad now skips to the next song in the menu screen.
* Added new image sources, menu based ones, analogous to the entry ones... for the current menu:
  Menu
  MenuNamed
  MenuBase
  MenuBaseIdx
  (These are untested)
* FTPServer was "slightly rewritten" - J.P.Patches core was great, I just re-did it using a bit 
  more robust string object (no overflows) and added XML configuration. Some testing was done, 
  looks OK. PASV mode still going to be an issue.
* New Node in Text Gadget <Multi> to support compound strings
  Example:
  <Multi>
   <Text>This is </Text>
   <Text source="disctitle" />
   <Text constant=" more text" />
   <Text const=" this also acceptable " />
   <Text source="configvalue" section="information" key="UserName"/>
   </Multi>
  This will create a compound string that combines the values.
* Fixed nasty little parsing issue - It didn't affect too much before, but arose under certain circumstances.
* XML values now support HTML Codes: &lt; &gt; &amp; for "<", ">", and "&" respectively.
* Added Time Synchronization. Supports network Timeservers (port 13) at the moment, more will be added.
  Preferences::SynchronizeClock (True/False, false if TimeServer is not defined, otherwise defaults to True)
  Preferences::TimeServerIP (Defaults to null)
* Added Network settings node:
  Network::UseDHCP (True/False, defaults to last setting, otherwise)
  Network::IP
  Network::DNS1
  Network::DNS2
  Network::DefaultGateway
  Network::SubnetMask
* FTP Priority setting: 
  Preferences::FTPPriority (Lowest/Low/BelowNormal/Normal/AboveNormal/High/Highest Defaults to Normal)
* Made improvements to speed on FTP uploads and downloads
* Tweaked startup to use the undocumented XeImageFileName to determine the startup path. This should fix
  a few problems, hopefully. 
* In the skins, multiple image resources can be randomly selected from at bootup. See the skinning guide for details.  
  (This is untested)
* If no items are found in the MXM.xml as subnodes of "Config" then it will also look for a "Menu" subnode as second chance.
* Added System Information screen - It's not too exciting for now, but it does consolidate much of the info available.
* Fixed issue with FTP and root directories.

Beta 0.9m
* Fixed FTP Issues:
  - Can now properly overwrite files
  - Corrected issue with some FTP Clients not displaying directory listings
  - RMD should work 100% now
  - Directory Listings clone Evo-X output
  - PASV now works (some testing done)

Beta 0.9n
* Fixed problem with "Bounce" modifier that caused the size to be screwed up for
  dynamic strings when they changed.
* Implimented <SkinsPath> for <Main> node. Defaults to MXM path, but this will override the value and 
  search will occur in the path specified in this element.
* Implemented FTP SITE commands:
  SITE reboot
  SITE shutdown
* Added <DuplicateOK> property to item entries. If True, it overrides check for duplicates. False has no effect.
* Fixed some issues with the FTP Server, works with AceFTP 2
* Fixed XML "cooking" - use of escaped symbols ampersand, greater and less than. Tested with Toejam & Earl
* Added "FlashID" Text String (smell anything burning?)
* Added Screenshot capability!!!! RIGHT-TRIGGER and "X" will capture screen captures to Z:\\
* Added X:, Y:, and Z: to the FTP Served drives.
* Added internal images: ::ICO_ACTION, ::ICO_DIR, ::ICO_DVD, ::ICO_EMU, ::ICO_FILE, ::ICO_HD, 
  ::ICO_LOCK, ::ICO_MEDIA, ::ICO_MUSIC, ::ICO_WAVE, ::ICO_XBE
* Internalized a master XML script that will hold internal skins, infoscreens and various other things.
* Added masahiko's "Simplicity" skin to the internal skin list.
* Skin Selection... not terribly stable, due to a number of factors, I am forcing a reboot after you select
  anything but "Random" - The changes go into MXM's "User" directory (U:\Pref.xml or E:\UDATA\00004321\Pref.xml)
* Some Newsfeed capability...
  Add a Newsfeed node into the Config node of MXM.xml:
  <Config>
    :   :
   <Newsfeed name="user">
    <Global>True</Global>
    <RotateChannels>False</RotateChannels>
    <URL>http://www.xbox-scene.com/xbox1data/xbox-scene.xml</URL>
    <UpdateIntervalMin>28</UpdateIntervalMin>
    <RotateItemInterval>10</RotateItemInterval>
   </Newsfeed>
    :   :
  </Config>
  Access it with a text source of "newsfeed"
  The user MUST have a proper DNS defined, otherwise, they have to use the dotted 'numeric' IP for the server.
* Added status indicator to MatrixEffect screen (also entailed new, more complete font; as a consequence, 
  the internal Matrix Font is usable now... lowercase alphas OK, uppercase are Matrix Symbols)
* Added Oswald's "Uyenbox3" skin to the internal skin list.
* Changed "Reboot" to full Power Cycle
* Added "PowerCycle" to menu options (SITE command)
* Added new attributes to AutoDir:
  "NoDemo" - If "True" will prevent AutoDir from including any searches into directories with 'demo' in name
  "DefaultOnly" - Prevents AutoDir from seeing anything but "default.xbe" files when searching for xbe files
                  (This does not affect directories with MXM_Entry.xml files)            
* Corrected filetimes in FTP Server to LOCAL TIME!!!!
* Fixed DateFormat bug. Added new formats for Time and Date:
  Time - 0=24 hour clock HH:MM:SS
         1=12 hour clock HH:MM:SSxm with AM/PM indicator
         2=24 hour clock HH:MM
         3=12 hour clock HH:MMxm with AM/PM indicator
  Date - 0=American, MM/DD/YY
         1=Euro1, YY/MM/DD
         2=Euro2, DD/MM YY
         3=Euro3, DD/MM/YY
  TimeSepChar and DateSepChar will replace the ":" and "/" respectively.
* FINALLY! Fixed the issue with no-audio WMVs. It seems it WAS my fault after all. Well, not entirely... M$ is a
  bit lame on how the routine I was using worked; so after a bit of re-working, videos should work both with and
  without audio.
* Fixed problem with Pref.xml not being saved, if you never ran MXM as an app (only dash) - the UDATA and TDATA
  directories were never created. This is now checked on startup and these directories are created.
* OK, found the bug preventing Command Lines from working. Try again...
* Finally! Alpha  sorting! Current WIP is defaults to alpha sorting, it should cascade to all submenu levels.
  <Sortfield> tag in an item begins the sorting (you cannot next these) which continues until the end of the entries or an <EndSort> tag
  is encountered.
* Alpha Sorting has been expanded to support up to three sort fields - just use additional <SortField> tags
* Preferences menu has been beefed up.
* Additional AutoConfig type added... "Apps & Games" set by the Preferences menu.
* Fixed IR Remote operation in system menus
* Fixed Random Soundtrack using a randomized index table (Shuffle play).
* Fixed "pam" in time (She didn't need to be in there anyway...)
* Fixed Video Standard String
* Added <PasscodeMD5> tag to item entries... used INSTEAD OF <passcode>  if additional security is desired.
  It's the MD5 hash of the 5 character alphanumeric uppercase passcode
* Added ability to select video standard from preferences/local menu
* "SAFE MODE" - if initialization fails on startup, rebooting should result in a safe mode menu. This is pretty straightforward from there.
  You can initiate "Safe Mode" by pressing "START" during initialization.
* <videomode> tag in entry allows you to set NTSC or PAL, IF you set "Multi Standard" in TV Type Preference setting.
* Added "SITE VIDEO PAL" and "SITE VIDEO NTSC" as recovery commands available using the FTP Server.
  These will set the video and reboot the Xbox. Use these if your display gets hosed completely (then heed the warnings!).
* Added "No Music" option to safe mode choices
* Reworked a couple things on the soundtrack manager
* Work done on FTP Server. XBRenamer now works (Found bug in RETR code)
* Fixed issue with FTP Server when moronic client attempts to create an existing directory (succeeds now)
* Added "CGEntryValueDWORD" string source, to report values as 10 character, 8 digit, C-Style hex values (0x00000000)
* Added "CGEntryMode" to report the desired video mode
* Added <WordWrapEnable> and <WordWrapWidth> to Text Elements - scaling is based on this width (implied), and fonts are displayed proportionally.
* Fixed "bug" that caused seeming random selection in the root menu (legacy from DVD Menu days)
* Better "Game Info" screen.
* Added "Simple Init" screen (See MXM_Patcher)
* Added Color Selection, good for Matrix and Simple Init Screens (See MXM_Patcher)
* DVD Compilations have several options to alter the behavior of the skins loader:
  Main::UsePrefSkin (Defaults to FALSE) If TRUE, uses setting in Preferences menu
  Main::RandomSkins (Defaults to FALSE) If TRUE, and in XDI/MenuX compatibility, allows randomizer to pick skin
  Main::DVDSkinsOnly (Defaults to TRUE) If TRUE, prevents usage of internal skins, forces skin searching to D:\ 
* NOTE: Renaming failures in FTP are due to open files in those directories!! I'll need to come up with something
  to work around this limitation. It is an unfortunate side affect of having a feature-rich skinning system :(
* The USer's guide needs some work... expect updates soon on the MXM tutorials page

I tested some word-wrap code as well, for true multi-line support, but I need to think on a few things to correctly implement this in the Text Gadget.
The word-wrapping function is working, so as soone as I determine the implementation details, skinners will have access to it.


Beta 0.9n.1 (Bug Fixes)
[*] Extremely bad situation with PAL/NTSC mode switching affecting people's systems. Now disabled until further notice.

Beta 0.9n.2 (Bug Fixes)
[*] Fixed FTP reconnection issue (non-random ports generated for PASV - "rand()" wasn't working)
[*] Fixed Xbox Version not showing up for some people.
[*] Dunno... newsfeeds weren't cycling. I suspect a bug in the compiler optimizations - it suddenly started working.
[*] Hmmm... changed some of the newsfeed stuff. Should  work on more feeds?
[*] Added a bit more game information (Content Size)
[*] Added Music Volume to preferences
[*] Moved DVD menu option
[*] Added the ability to screenshot in menus and messageboxes (Right-Trigger + "X")
[*] Added a few more Preferences options (Saver delay, "Remove" menu options)
[*] Re-arranged the skin loading a bit to fix a couple minor issues (seems to be faster now, too)
[*] Fixed bug in AppMenu (which drives the "Y" System Menus) that sometimes prevented a submenu from working.
[*] Added "Emulators" to the Apps and Games style auto-config
[*] Clarified options specific to "AutoCfg" menus
[*] Added No Demos and Default.xbe options
[*] Fixed (I hope) issues when global soundtracks were uninitialized on the Xbox
[*] Added "Use Internal Skins" Option while re-arranging some preference menus.
[*] Added <MusicEnable> tag to Layout nodes in skins (to disable music in a layout)

Beta 0.9n.3 (Bug Fixes)
[*] Changed a little bit about the way the menu cache works.
[*] Removed "B"/Back to activate the System Menu (Use only "Y" now)
[*] Fixed issue with SortFields being used with AutoDir (not propogated to cache menu)
[*] Newsfeed "sticking" issue seems to have been fixed, finally (didn't happen in Debug).
[*] Fixed problem with newsfeed updates retried almost continuously when they fail.

Beta 0.9n.4 (Bug Fixes)
[*] Added <FadeIn> tag to layouts, which prevent the fade in if set to FALSE
[*] Fixed issue with Alpha-masked PNG files not displaying properly
[*] Fixed another issue with Alpha on Fonts not being displayed correctly!
[*] xISO xFER now works with MXM (tested on 2.02) - xFER didn't like my MKD response code.
[*] New SITE commands:
  SITE ResetMenuCache
     FTP Client apps could refresh the menu selections
  SITE trayopen
     Opens Drive Tray
  SITE trayclose
     Closes Drive Tray
[*] Oh, just for kicks, I added a "System..." submenu and implemented the "Action" tag for menu items.
  This is pretty simple, at the moment, but I'll build this up as time progresses.
  The test menu features "Tray Eject" and "Tray Close" as well as reboot options.
[*] Added conditional "itemaction"
[*] Fixed small problem with the Time Synch not being disabled when the network was disabled by SAFE MODE
[*] Tweaked the menu loading again.


Beta 0.9n.5 (Bug Fixes)
[*]  Fixed Music Enable
[*] Checked Enable Global Music - works on my test machine
[*] Added "Enable Network" to configuration menus.
[*] Explaination of the <Action> tag to add your own "Systme" commands to the 'regular' menus:
  Instead of defining an "Exe" You define an Action for the item:
  <Item>
  <Action>trayopen</Action>
  <Title>Open DVD Tray</Title>
  <Thumbnail>::ICO_ACTION</Thumbnail>
  <Media>::ICO_ACTION</Media>
  </Item>
  Actions currently available:
  - ResetMenuCache
  - trayopen
  - trayclose
  - reboot
  - shutdown
  - PowerCycle
  More will follow
[*] Totally new FTP server code. Should be no more hangups... you can now ABOR during transfers.
[*] Recognizes "G:" drive
[*] Toggle for "Action..." menu (Formarly "System..." but confusing name with "Y" menu)
[*] Hard drive Formatting!
  - SITE commands:
     FORMAT C:
     FORMAT E:
     FORMAT F:
     FORMAT G:
     FORMAT X:
     FORMAT Y:
     FORMAT Z:
   Of course, use these at your own risk!!
   As usual, these SITE commands can also be used in ACTION menu items
   Requires a toggle of the configuration menu (Misc settings) before first usage.
[*] Drive-related Strings have "G" equivalents now.
[*] Conditionals: cdrivepresent, edrivepresent, fdrivepresent, gdrivepresent
[*] Added "not" attribute to conditionals, for testing OPPOSITE condition. To use, set not="true"
    i.e.: <condition not="true">GDrivePresent</condition>
[*] Added "fanspeed" gadget string.
[*] Added
    ATTR <FileOrDir> ro|rw
    CHMOD 777 <FileOrDir>   (This command is compatible with FlashFXP's "Attributes" action)
    LaunchDVD
    EEPROMBackup [Filename] (Backs up EEPROM contents, encrypted, to a file, file defaults to "eeprom.bin" in current directory)
    EEPROMRestore [Filename] (Restores EEPROM contents, encrypted, from a file, file defaults to "eeprom.bin" in current directory)
    MKDIR <PATH>
    RMDIR <PATH> <CASCADE>
    CD <PATH> (GUI only, sets default directory, which starts as MXM's path)
    COPY <Path/File/Wildcard> <Path/File>
    RENAME <FileOrPath> <FileOrPath>
    DELETE <File>
    :<Label>
    GOTO <Label>
    SYSTEMMENU <Activate|Hide|Unhide|ActivateFull>
    MESSAGEBOX <message> (Displays a message box AFTER script is done executing, or if used between BeginDraw and EndDraw, renders a message box)
    SET <VarName> <Value>  
    ADD <VarName> <Value>
    SUB <VarName> <Value>
    DIV <VarName> <Value>
    MULT <VarName> <Value>
    IF <arg1> <op> <arg2> goto <label>
    -<op>:
     - =
     - >
     - <
     - >=
     - <=
     - != or <>
     - ##   (True if LValue contains RValue)
     - #^   (Same as ##, but case insensitive)
     - !##   (True if LValue doesn't contain RValue)
     - !#^   (Same as !##, but case insensitive)
     IF# (Used to evaluate numerically)
    SETFANSPEED <Manual|Default> <speed>
    SETFUNC <VarName> <Function> <Function Arguments>
     -Functions:
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
    BEGINDRAW [USeCurrent]    (Starts rendering a screen, "UseCurrent" grabs the previously rendered screen to display)
    ENDDRAW   (Displays screen)
    INPUT    (See ActionScript ReadMe for details on return variables set)
    CALLFILE <file> <Args...>
    CALLSCRIPT <name> <Args...>
    BOX <X> <Y> <Width> <Height> [<Color>] [<BorderColor>]
    TEXT <X> <YTOP> <CENTER|LEFT|RIGHT> <Text> [<Color>]
    ALLOWFORMAT <1|0>
    MOUNT <Drive:> [<DevicePartitionPath>]
    DATE <Day> <Month> <Year>
    TIME <00:00>
    DELAY <Delay Sec>
    DELAYMS <Delay ms>
[*] Add "CherryPatch" - Yes, Thank you Cherry for your PAL/NTSC code. It's EEPROM safe, so you should be OK with a simple reboot.
    With this patch, the PAL/PAL60/NTSC switching code is reimplemented!! It's improved, though, as there is now support for PAL60
    and USA/JAP/EUR region coding as well.
[*] ActionScripts can be loaded internally from the MXM.xml file - define them in a <Scripts> node, with each script:
<ActionScript name=somename">
...script here...
</ActionScript>
and
<ActionScriptLoad name=somename">loadable.xas</ActionScriptLoad>
Loads form the disk, relative to the MXM.xml file
[*] MXM will execute "AutoExec" scripts - 
1) First, it runs an internally defined script, _AutoExec
2) It runs a user-defined script, AutoExec, as defined in the MXM.xml
3) If it exists, it will run "AutoExec.xas" from the MXM directory.
[*] Colors can either be the named versions, or ARGB values (0xFF001122, for example)
[*] Special ActionScript symbols, $eol$, $lt$ and $gt$
[*] Fixed passcode usage with SubMenus and ActionScripts. Also cleaned up rendering
so the passcode is not revealed once passed by the fade-in.
[*] Cleaned up the way the Dashboards menu option works. Much nicer now ;)
[*] Added "DVD Player Preference" to select a preferred player (Default, X2, or M$)
[*] Users can add "Context menus" for games. These are actionscripts defined in a <Context> node in MXM.xml - 
The scripts have some awareness of the currently selected game, and can act on them.
[*] Added "TimeSynch Offset" in hours for those using non-local time servers.


Beta 0.9n.6 (Bug Fixes)
[*] Added experimental automatic G: fix
    If drive F: is not between 128GB and 132GB, MXM will not automatically mount G:
[*] Failing that, there is now an MXM_Patcher option to force MXM not to init G:
[*] SETFUNC::FILESIZE should now return directory size now.
[*] New ActionScript commands:
    BEGINSEARCH <WildCardPath>
    - The local variables "search_name", "search_path", "search_arg", "search_size", "search_type", "search_active" (which equals "1" when the search values are OK) are updated during the search
    SEARCHITEM <VarName>   (Load VarName with current file/pathname)
    SEARCHSIZE <VarName>   (Load VarName with current file size)
    SEARCHTYPE <VarName>   (Load VarName with current type for entry - "file" or "dir")
    SEARCHNEXT
    ENDSEARCH
    MEDIAPATCH <FilePath> (Experimental, patches media flag and media check code)
    INPUT [NOWAIT] (Added optional "nowait" parameter, which prevents it from waiting forinput)
    OPENREAD <HandleName> <FilePath>
    OPENWRITE <HandleName> <FilePath>
    READFILELN <HandleName> <VarName> (Reads line and loads variable)
    WRITEFILELN <HandleName> <Text> (Writes line with text and appends CR/LF)
    WRITEFILE <HandleName> <Text> (Writes line with text)
    CLOSEFILE <HandleName>
    FILESTAT <HandleName> <VarName> (If file access is OK, value will be "1" - for read, "0" means end - otherwise "")
    WEBFILEFETCH <Filename> <URL>  (A little goodie... but this is a very basic HTTP client, so it won't work with all sites)
    SETFUNC new:
     - FNBASENAME <FilePath>
     - FNEXTENSION <FilePath>
     - FNPATH <FilePath>
     - FNMERGE <Path> <Filename> [<Extension>]
     - FATXIFY <filename> [<replacementchar>] (Do not use with a full path! backslashes and colons will be removed!!)
     - TEMPFILE <Path> <Prefix> (Creates a new temporary filename)
     - REPLACE <Old> <New> <Source>
     - ContextScriptExists <ScriptName>
     - ScriptExists <ScriptName>
    XMLOpen <HandleName> <FileName>
    - Opens an XML file or internal XML link (::MXM, ::PREFS, ::MENUCACHE, ::MENU, ::Internal)
    XMLSave <HandleName> [<FileName>]
    - Saves XML back out to a given file
    XMLSaveNode <HandleName> <FileName>
    - Save current selected node to file (File must not exist)
    XMLSetNodePtr <HandleName> <Node> [OnFail <Label>]
    - Set the current node pointer
    - To start from "root" use ! Example: !.FTPServer.User:1    (Accesses second user node in FTPServer node of a config file)
    XMLCreateNode <HandleName> <Node> [OnFail <Label>]
    XMLCreateElement <HandleName> <Element> [OnFail <Label>]
    XMLSetElementPtr <HandleName> <Element> [OnFail <Label>]
    - Must be relative to currently set node, or this will also cause the current node to change
    XMLSetElementValue <HandleName> <Value>
    XMLSetElementAttr <HandleValue> <Attribute> <Value>
    XMLSetNodeAttr <HandleValue> <Attribute> <Value>
    XMLSetValue <HandleValue> <Location> <Value> [<AttrPref>]
    XMLGetValue <HandleValue> <VarName> <Location> [<DefaultValue>]
    XMLGetNodeCount <HandleName> <VarName> [<Name>]
    XMLGetElementCount <HandleName> <VarName> [<Name>]
    XMLGetNodeAttrCount <HandleName> <VarName>
    XMLGetElementAttrCount <HandleName> <VarName>
    XMLCreate <HandleName> <MainNodeName>
    LoadCurrent
    CallContextScript <ScriptName>
[*] Fixed issue with submenu conditional
[*] ActionScripts now run in their own thread, meaning the video and audio continue while ActionScripts run. This will be further exploited in the future with scheduled and event-driven execution.
[*] Fixed File access error in ActionScript FILE functions.
[*] Added new Text Gadget source: ENV - this gives the skinner access to the global environment (Environment variables prefixed with the "_" character)
    Define the "Key" attribute to select an environment variable to display (without the % on each side, with or without the underscore)
[*] Rewrote the ActionScripting engine. This will be getting a slight re-arrangement when I incoroporate the scheduler, which is more or less written.
[*] Added $timer$ 'special' ActionScript variable, which gives a running count of milliseconds
[*] Added _GP_ANYBUTTON variable set by input for simple testing.
[*] Fixed non-working string functions and issue with XMLSetValue when setting attributes
[*] Fixed spurious file entries when connected to M$ Internet Explorer via FTP. This might fix other issues (I hope).
[*] Added DNS cacheing. This may solve some issues with newsfeeds, but if edited, the cache file can make DNS lookups more efficient!
    In the file, you can set a hostname as one of three types of persistence: dynamic (always looks up IP), session (Looks up IP once per session) and static (you can 'permanently' assign an IP)
    If the cached IP doesn't work, MXM will try and get another one, no matter what.
[*] Changed the FTP Server again.... should be working again.
[*] Added jlm5151's Simplicity Remix skins as internal skins.
[*] Added more specials: $tm_time$, $tm_date$, $tm_hours$, $tm_minutes$, $tm_seconds$, $tm_year$, $tm_month$, $tm_day$ - all based on current time. Format for date: DD/MM/YYYY and time: HH:MM:SS. Individual units are 2 digits (or for year, 4 digit)
[*] Fixed issue that prevented "RandomSource" enabled image resources from working properly.





=============================NOTES================================
=============================NOTES================================
============Things not done yet, but to come soon....=============
=============================NOTES================================
=============================NOTES================================




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



FileManager

Display screen in system colors...
--------42 chars----
  ..				01-OCT-02 <Dir>
[]Dir1 				01-OCT-02 <Dir>
[]Dir2				01-OCT-02 <Dir>
[]File1.XBE			20-APR-03 109782

24.5MB in 12 Files/Dirs Selected 
E:\Games\KOTOR

Mark/Unmark (Right-Trg for all)
Activate (Launch or View)
Context Menu
Cut
Paste
Rename
Delete
Properties
Toggle Sides

DVD-R menu version, stripped down:

Handles skins
No FTP Server
Limited actionscripting
No deep searching
No Newsfeeds
No Time Synchro
No formatting
No BIOS Flashing



To do:
* Actions:
  LINE <X1> <Y1> <X2> <Y2> <Color>
* Check using 1st and 4th game ports
* Add check to "install" metadata info in Title Directory
* Multiple Newsfeeds... something wrong?
* Clean up image cacheing
* Check Action conditional
* Insert User Strings into internal skins
* User-definable description and title fields for autocfg items
  language.xml
  <AutoCfg>
   <EDrive>
   <FDrive>
   <Dashboards>
   <System>
   <SubMenuDescr>
   <SubMenuDescr>
* Check Game Help Screen stuff (Still seems working?)
* Add <ZFactor> tag to image elemnts
* Add <AlphaBlend> tag to image elements
* Add <AlphaMask> tag for alpha mask gray-scale images on Image Elements
* Named colors, to consolidate them into a standard reference.
* Check against SecureFX (Seems to work fine, if slow)
* More Preferences:
  *FTPPriority
  *AllowAnon
  *EnableFTP
  *UseTimerServerSynch
  *UseInternalSkins
  *Saver Delay
  UseMenuCache
  MoveDelay
  RepeatDelay
  *UseMusic
  *Default.xbe Only
  *Mask Out 'Demo'
  *RemoveInvalid
  *RemoveDuplicates
* Preview support for skins
* Support for multiple <Media> (or any) tags in entries, and ranomly choose between them?
* Delete Game Entry option ("uninstall game")
* Keep copy of menu in memory SEPARATE from regular menu, so we can edit it and save out cache or rebuild it!!
  - Allow entries to be deleted, based on XBE path
  - Allow entries to be added
* Possible bug in X Scaling of MXM menu blocks when including a thumbnail.
* Screenshots in Loading? Title Image?
* ? Make Safe Mode Menu IR Compatible?
* Add Phoenix Loader code to allow users to select a BIOS and load it in the BIOS Shadow.
* Build Installer into MXM (install from DVD)
* Game Ripper
* Add audio and music volume controls in the preferences menu
* Add string source "user", where key=index value, 0-999, and "default" is default text source.
  Must be simple source string.
* Fix Shifter
* Double check videomode and region checking on games
* Fix issues running MXM as DVD-R menu'd disc from Evo-X autolaunch
* Add better Game Info:
  Region Code
  Built-in Description and Publisher
  Path to game
  FATX Compatible naming
* Sound triggers for specific game titles?
* MXM_Data.xml tag, perhaps, to identify the app used for a directory of data files?
* Notify FTP users the system is disconnecting
* Check DVD Mode ONCE again.
  Seems to be related to Evo-X running the app directly, as opposed to letting the system do it via a reboot
* Insure proper file closure if user aborts FTP session....
* Integrate "MXMBasic" - a derivative of TinyBasic as the scripting language. Another MXM First!
  This language will be expanded to do all sorts of cool things too, perhaps even limited abilities to handle
  input and images so simple games can be written and such. ;)
* File Manager mode for menu screen
  Top Level: Drive letters
  Description would have details on drive, directory or file
  Title would be file or directory name
  File Folder, Sheet, or Drive symbol would be built in images.
* Add conditional for File Manager Mode
* SITE AutoRenameOn / AutoRenameOff (Renames to legal FATX on Xbox end)
* Skin screen adjustments
* Filename Validity check for FATX, XDFS and UDF
* Add variable buffer size setting for FTPServer
* Add Scrolling modifier for text
  <Modifier Type="scroll" Target="HorizontalOffset>
   <Delta>-1</Delta>
   <Gap>10</Gap>
  </Modifier>
* Event Triggers for sounds
* Random event timers
* Slave timers
* Add playlist capability for image elements?
* Add music modifier
* Detect Audio Discs
* Read CDDA tracks using IOCtl
* Add ability to synchronize with atomic clock server
* Add Flash BIOS code
* Add Dialog/Control Classes
* Master Thumbnail and Media directories to pull images/movies out of. How will it work? Image will be named with one of the following methods:
- tn0x00000000.* where the base name is the hexadecimal representation of the game's XBE ID
- tn99999.* where the base name is the decimal representation of the game's XBE ID
- tn(xxx).* where 'xxx' is the XBE embedded name with underscores replacing any spaces.
'tn' prefix is for Thumbnails, 'pv' for preview media. Of course, the extension can be bmp, png, tga, jpg or wmv.
If there is no specific media or thumbnail entry for an item, MXM should access the appropriate master directory and use the image there, if it's found.
This will let you put a large chunk of your media in one place
* Check <LoadSkin> again. Seems to work fine
* Rename Directories doesn't work? CHECKED. Works fine. Probably didn't work if used on Skins Directory
* Game Info not working within submenus? CHECKED, works fine.


* Verify settings work as advertised
* Allow some settings to be configured from system menu

Configuring IP:
 DHCP or Static

Music Visualization will be done in a similar manner to WMP:

Two 2-D arrays of ints

frequency[2][1024] 
20-22020





Start up behavior:

System gets launch parameters
Determines which drive it was launched from:
 If Launched from the DVD, it's in DVD-R Menu Mode
 ELSE If D: does not exists, symbolically, it's in Dashboard Mode
 ELSE If D: Exists as a Hard Drive Path, It's in Launcher Mode
It then mounts all of the logical drives
If Z:\safemode.txt exists, "Safe Mode" is flagged
Options are loaded from patch location
EEPROM info is read
Application Framework is initialized

If SAFE MODE
  Display Menu - Give Choices
  A - 

Intialization screen is displayed
	If In DVD-R Mode 
  If XDI config exists AND MXM.xml does NOT exist, XDI Mode
    Load XDI info
    Generate internal XDI Skin
    Load Entries from XDI config
  Else
    Load Entries
Else
 Load Entries
Load Skins






FATX
UDFX

!#$%&'()-.@[]^_`{}~0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ
 
Numerals 0 through 9 
Upper and lower-case letters A through Z 
Character values 128 thru 255 


====== Can Wait For Later =======
14. Consolidate init screen code
13. Create one or two new init screens
4. Add arrow gadgets	
5. Poly Gadgets

New Options:
<AutoDir Flatten="true">E:\</AutoDir>

<RemoveDuplicates></RemoveDuplicates>



Text Fields to support:
*Current Game Title
*Current Game Descr
*Disc Title
*Disc SubTitle
*Disk Free Space
*Disk Total Space
Disk Used Space
*BIOS
(Find out all the Evo-X supported strings and add them)
*CD (Disc inserted status?)
*Time*
*MXMVersion
*KernelVersion
*BiosVersion
RDName
*Name
*Temps
*Date
*DateTime
*IP
*ConfigValue (Section, Key)
*SkinValue (Section, Key)
*CGEntryValue (Section, Key)
Ethernet Line Status
IP Type
*Subnet Mask
*Default Gateway
*DNS1
*DNS2
Xbox Hardware Info
Flash Type


Number of items in current menu
Number of total items
Number of items in submenu selected


Loading Entries:
Each Xbe Entry will perform the following checks:
- File Must Exist (Unless "Debug" is on)
- If No Title, one is grabbed from the Xbe
- If No Description, One will be generated (Unless disabled via config)
- If No Media Entry:
  Searches for MXM_SS.xxx files. WMV first, followed by JPG, BMP, TGA, and PNG
  Uses TitleImage.xbx, if available
- Creates a "Thumbnail" Entry for TitleImage.xbx file (uses Media if not available)  






The Kernel Sits in Memory at start address of 0x80010000 You can try and patch some of the unused .EXE headers in RAM, shutdown your app, run a new one and inspect the changed area to see if the change persisted.. It theoretically should've.. Havnt tried it yet, but this is like 5 lines of code to test it..

not sure where you whould put your "igr" interrupt service routine and where you would "hook" it in, but thats at least a start...




This release is intended to bring the new skinning technology to AT LEAST the level of the old skins,
and compatibility will be maintained (we will load the old skins in and rebuild them internally as
new skins). Many enhanced features will also be available. See the SkinReference guide for more details.



KNOWN ISSUES/TO DO LIST:
* Add polygon definition for Image Gadget, so they can be displayed in a non-rectangular region.
* Add Yes/No dialog to app toolbox.
* Apparently, MXM cannot be launched from BoXplorer. It hangs on the call to DirectSoundCreate()
  only a few lines into initialization. If anybody knows why this is happening, please clue 
  me in. Tulkas... if you read this, CONTACT ME.
* Users have expressed a desire to see the screenshots fade from one to another.
* Items loaded from the MXM.xml are not checked yet for validity. If they don't exist,
  the entries should be tossed.
* Support for XMV is not present.
* More styles needed (this is beyond the skinnable "simple style", referring to 3D styles
  like a spinning screenshot cube, for example.)
* Circular menus option
* Get default sound into xbe
* Update smooth scrolling to work for circular and horizontal options.
* Add thumbnail screen
  Displays "x" entries in HxW format on screen
  Skin defines 'box', and number of entries H and W.
  Define key behavior (Up/Down/Left/Right)
* Helper Win32 apps needed for HD configuring and another for validating DVD layouts.

===================================================================
Dashboard Enhancement Project
===================================================================
OK, it's really a wishlist of features I'd like to see included to offer 
fullfledged dashboard support in MXM:

* FTP server
* HTTP Server
* Telnet Server
* Executable Scripting language 
* VNC Client
* VNC Server 
* BIOS Flasher
* EEPROM config manager
* CD Music Ripper
* Save game management
* Xbox-to-Xbox transfer util with CRC file checking
* Installer
* Filemanager
* Instant Messenger client
* IRC client
* Voice/video over IP
* DVD ripper  (Seperate application, launched by MXM)
* Audio Media Player with visual effects
* Graphics slideshow display
* E-Mail client
* Defrag utility (I'm kinda iffy about this, as to need, but I've had a killer idea to do this FASTER than anything before it)
* Time setting Manual and 'atomic'
* Format and HD Prep

Other goodies:

* Flash viewer


References:

http://www.uk.research.att.com/vnc/faq.html

===================================================================
Things that couldn't make it into this release, but are planned:
===================================================================
* More detailed directions. Look, this stuff takes forever, and this will
  have to do until the later. Sorry.
* XMV and other animated format support. WMV is supported for now, though.
* Smarter relative paths. For now, we must assume any relative path is a CD/DVD. There
  is still work to be done to examine symbolic links so the menu system knows "where" it 
  actually  is being run from.
* More eye-candy. 
  - Animated menus (M$ Dash style or a rotating screenshot cube, for example)
  - Soundtrack-aware visuals in a screensaver
* Support for all video modes (HDTV, PAL).
* Full support for external fonts.



===================================================================
Other ideas:
===================================================================

* A PC-side app to handhold through the process of making and verifying a menu.

===================================================================
Suggestions:
===================================================================

[maidenman from xbox-scene]----------

 How about waiting five seconds and then automatically booting that game. (Similar to the Windows multi-boot  menu). This way, those who use Media X as their dashboard can have their current game of choice boot  automatically, no user-intervention, when they turn on their Box! This should be an option than can be  disabled in the .ini.

 If you want to get fancy, remember how many times every game on the menu has been loaded, and re-order the  menu so most popular games are on top, the way the Windows XP start menu works. Those games you have on the  Hard Drive just for the occasional demo for friends will automagically be positioned on the end of the list.

[weavus from xbox-scene]----------
 Too many to count. :) I'm trying to get as much as possible in 1.0

 I think we are going to need the ability to turn off the admittedly nice new loading sky/logo as it looks out of place when you are using nested MXM's. I.E. having one as a main menu and sub ones for categories. I like the fact it displays quickly but would rather have the title screen while I wait for a sub MXM to load.  

>>My response<<
What I might do, when I get it working, is have the initialization screen (which is what's happening when it's displayed) disabled when it loads with parameters... which is how I might handle using multiple MXM.xml files. I'm still tweaking that particular issue in my head.

The title screen is a bit difficult to throw up, since a lot has to happen before we can even get to that stage (including loading & parsing MXM.xml, looking for skins, loading and parsing the MXM_Skin.xml, and loading the associated image). It's much faster now (a few seconds) and believe it or now, image loading is actually QUICKER when using WMV videos. I'll play around a bit more with it.

I might also make an optional "fade to black and back" transition between states (actually, there's probably a whole slew of transitions I could add now).



[NetDan]----------------
 How about adding game categery menus? It would be great to organize the games like (FPS, Racing, Action, Sports, etc). I know I can do this with EvolutionX but I really like MenuX because I can see game covers and screenshots. I don't know how hard this would be but thanks in advance if you do decide to add this feature!


===================================================================
Configuring MXM:
===================================================================
Configuration is now done with XML files. This was done to maintain 
consistency, since the upcoming features require the flexibility of 
XML for more Generic skinning options.

For the most part, you only need to run the INI to XML converter for 
Windows (released with these binaries) to convert your existing files
over to the new file format. You may even edit these files as INI 
files and convert to XMl with the tool.

XML is pretty straightforward, looking much like an HTML file. For
MXM configuration, the files will look something like this:

<Config>
<Section>
<Key>value</Key>
</Section>
</Config>

This format will also allow multi-line descriptions in game entries
which will be very useful in the near future.

Well, enough discussing the merits of XML, on with the show:

We won't be detailing skin options. These should be set from the Skin
configuration file. From this version on out, such options are 
"officially" unsupported in the main MXM.xml file, and while they 
may work at the moment, they may not in the next version.



-------------------
Menu items:	
-------------------

Each menu item will be listed in its own section, using "Item_x" as the name.
It will read each item in order, starting with Item_1, until it fails to find 
the "next" one.... See the sample INI file for a detailed example.


Example:

<Item_1>
<Title>ToeJam and Earl III</Title>
<Description>Platforming classic returns!</Description>
<Media>TOEJAM3.png</Media>
<Dir>TOEJAM</Dir>
</Item_1>


Title
[title string]
  The title that's displayed in the menu selection

Description
[text to be displayed at the bottom of the selection menu]
  The text displayed at teh bottom of the menu selection box

Media
[filename (may include absolute or relative path) of image]
  The bitmap (BMP,TGA,PNG,JPG) used to display a screenshot or title screen

Dir
[Relative or absolute path to program]
  The directory the exeutable resides in

Exe
[executable filename]
  The executable filename, defaults to default.xbe

Passcode
[5 letter alphanumeric passcode]
  User must enter this code in a special screen before they are able to run a program.
  NOTE: This feature will not prevent a persistant user, but should work for most people.

HelpScr_(xx)
  (xx) - two digit index value from 01-99
[image filepath]
  If present, will enable game help for that entry (Black button on game controller). This is 
  handy for games that have complex controller setups, but can be used for other things as well.
  You may use WMV files here, too. MXM will search from 01, 02, ... until it hits a break in the 
  entries. These are accessed when viewing the help screen by pressing Left or Right when there
  is more than one help screen available. 
  (NOTE: Some people might want to toss the games video clips in here)

-------------------
Main config:	
-------------------


DiscTitle
[Title String Here]
  The title displayed in the title screen

DiscSubtitle
[Subtitle String Here]
  Text displayed below title in title screen

Style
[Style ID]
  Style ID, currently unsupported, when supported, this option
  will determine the menu 'style' used. NOTE: This does not refer 
  to the "skins" being used. 2-D skins are, as a whole, a style.
  It is unsupported, because in the future, MXM will have 3-D
  styles, in addition to the 2-D skinnable style.

*TitleDelay
[Delay in seconds]
  Delay, in seconds, for the display of the title screen on startup. (Default 20sec)
  Hitting a button will bypass the title screen. A value of 0 will disable the title screen.

*SaverDelay
[Delay in seconds]
  Delay before launching the screen saver from the menu screen. If "0" turns saver off.
  If greater than 0 and less than 30, it will set the delay to 30.
  (Default 300sec, or 5 min)
  
	
MXMPath
[Absolute path]
  Defaults to D:\, this represents the path the menu is being run from.
  Used to launch apps run from relative paths. This is only useful when running 
  the app from the hard drive, otherwise, just leave it off.


ExitApp
[Absolute filepath to dashboard]
  Because some are using MXM as a dashboard, this option was included to 
  facilitate the launch of a dashboard application using the "Dash" button
  combination (both thumbsticks down). Normally, this can be ignored.

ExitAppParams
[Text to feed to ExitApp application]
  Not currently used. 

LoadSkin
[Name of skin to use on startup]
  Technically, it's the name of the directory the skin resides in, for example 
  "LCARS" or "DefaultSkin" - a skin MUST exist in this directory for this to work.

AllowAutoConfig
[Yes/No]
  Defaults to Yes (if there are no item entries in MXM.xml), but if you set to No, 
  it will prevent MXM from searching subdirectories for games and applications to load up.

AllowAutoTitle
[Yes/No]
  Defaults to Yes (if there are no item entries in MXM.xml), but if you set to No, 
  it will prevent MXM from defaulting the game titles based on a preset value in MXM 
  using the game's unique ID code. This is automatcally overriden, if a game title is 
  specified for the entry.

AllowAutoDescr
[Yes/No]
  Defaults to Yes (if there are no item entries in MXM.xml), but if you set to No, 
  it will prevent MXM from defaulting the game titles based on a preset value in MXM 
  using the game's unique ID code. This is automatcally overriden, if a game title is 
  specified for the entry. If MXM does not have a unique description, it will attempt
  to use the publisher's name for the description.

AllowVideoSound
[Yes/No]
  Defaults to Yes. Pauses background music while videos arte played and plays the sound
  that accompanies the video. The reason it defaults to Yes is that it seems that when
  running videos on a DVD, it can sometimes bog down due to excessive disc accesses.

MoveDelay
[delay time in milliseconds]
  Defaults to 220 milliseconds

RepeatDelay
[delay time in milliseconds]
  Defaults to 220 milliseconds

EXAMPLE:

<Config>
<Main>
<DiscTitle>MediaX Menu</DiscTitle>
<DiscSubtitle>Subtitle here</DiscSubtitle>
<TitleDelay>20</TitleDelay>
<SaverDelay>300</SaverDelay>
<ExitApp>C:\evoxdash.xbe</ExitApp>
<LoadSkin>LCARS</LoadSkin>
<MXMPath>D:\</MXMPath>
<MoveDelay>100</MoveDelay>
<RepeatDelay>500</RepeatDelay>
</Main>
  :     :    :   :
  :     :    :   :
 Other sections
  :     :    :   :
  :     :    :   :
</Config>



-------------------
Menu config:	
-------------------

AllowSaverMusic=<on/true/1/yes or off/false/0/no>
AllowSaverSound=<on/true/1/yes or off/false/0/no>
  These are pretty much self-explanatory


Example:

<Config>
  :     :    :   :
  :     :    :   :
 Other sections
  :     :    :   :
  :     :    :   :
<Menu>
<AllowSaverMusic>on</AllowSaverMusic>
<AllowSaverSound>on</AllowSaverSound>
</Menu>
  :     :    :   :
  :     :    :   :
 Other sections
  :     :    :   :
  :     :    :   :
</Config>




-------------------
Help config:	
-------------------

Screen_(xx)
  Where (xx) is the screen index. There are three default help screens for MXM built
  into the system, but these can be overridden with this command. If any are defined here,
  they will prevent any of the built in ones from appearing.
  The screen numbering is sequential, from "01", until it reaches the last continuous 
  sequentially numbered item. You may witsh to use video files for help (sound is not played, 
  however).

Example:

<Config>
  :     :    :   :
  :     :    :   :
 Other sections
  :     :    :   :
  :     :    :   :
<Help>
<Screen_01>Help\GameControllerHlp.wmv</Screen_01>
<Screen_02>Help\IRControllerHlp.wmv</Screen_02>
</Help>
  :     :    :   :
  :     :    :   :
 Other sections
  :     :    :   :
  :     :    :   :
</Config>




-------------------
Music config:	
-------------------

Adding music tracks to the menu system

You have several basic options:
1. Playing a specific file
2. Playing a directory of music
3. Playing the Xbox's own soundtrack

You should always have something to play.

Options under [Music]:

UseMusic
[on/off]
  If on (Default), music will play, if off, no music will be played.

Background
[filename]
  This option allows you to place a single file into the "local" soundtrack. Use this
  to select a single file to play during the menu operation.

Directory
[directory]
  This option will cause the menu system to read an entire directory and add it to the 
  "local" soundtrack.

Global
[on/off]
  This option, when activated (default) will play any tracks contained on the Xbox's 
  regular soundtracks.

Random
[on/off]
  Default is on. This randomizes the order used to play the soundtrack files. If off,
  it will play the local soundtrack (in order) followed by the Xbox soundtracks (if 
  Global is on, of course)



<Config>
  :     :    :   :
  :     :    :   :
 Other sections
  :     :    :   :
  :     :    :   :
<Music>
<UseMusic>true</UseMusic>
<Background>MyTrack.wma</Background>
<Directory>MUSIC</Directory>
<Global>true</Global>
<Random>off</Random>
</Music>
</Config>





===================================================================
Skins:
===================================================================
  Eventually, there will be a more comprehensive guide on this. All I can 
suggest for the moment is to examine the current skins and work from those.
Remember that skins directories reside in the directory MXM itself is run
from. 


===================================================================
Directories:
===================================================================

On a DVD disc, it will look much like this:

D:\default.xbe   <-MXM.xbe renamed
D:\MXM.xml
D:\DefaultSkin
D:\DefaultSkin\*.* <-Default Skin files, including MXM_Skin.xml
D:\LCARS\          <-Another Skin, LCARS
D:\LCARS\*.*       <-LCARS skin files, including MXM_Skin.xml
D:\Media
D:\Media\font16.xpr
D:\Media\SkyTile.jpg
D:\Media\LargeLogo.png
D:\GAME1           <-Directory of a game
D:\GAME1\default.xbe
D:\GAME1\MXM_Entry.xml <-Game Entry configuration
D:\GAME1\MXM_SS.jpg    <-Screenshot (could also be a WMV, for example)
D:\GAME1\*.*           <-Remaining files and directories
D:\GAME2           <-Directory of a game
D:\GAME2\default.xbe
D:\GAME2\MXM_Entry.xml <-Game Entry configuration
D:\GAME2\MXM_SS.bmp    <-Screenshot (could also be a TGA, for example)
D:\GAME2\*.*           <-Remaining files and directories


On a hard disk, it will look similar, except you may specify the game entries
in the MXM.xml file, instead of relying on auto-config, for games not located 
immediately below MXM.

===================================================================
Note to developers:
===================================================================


Sorry this is in such a messy state.

I'd like to acknowledge CodeProject as the source of the CStdString class.
I've discovered that it really helps with the Xbox APIs, particularly with 
conversion, but it's a bit kludgy. Best to use CStdString natively (compiles 
as 8-bit ASCII strings) and whenever you need wchar_t types, simply assign 
to a temporary CStdStringW object and pass out "wsTemp.c_str()" for the 
desired wchar format. You could even use:

    CStdString sThis String(_T("test"));

	SomeFunctionW( CStdStringW(sThis).c_str() );

to pass in the value.

The INI classes were built specifically with the Xbox in mind, They utilize
STL templates for mapping the parameters and CStdString as the basis for
the strings. Feel free to port this to your own applications. I intended 
this implementation to support other formats as well, on multiple platforms,
such as XML and registry configurations on desktop systems or handhelds.
Support may be added for more data types, as well.

The application is laid out with the idea of operating the applications
in different states. Each state has it's own initialization, activation, 
movement and rendering functions. This makes it easier to bounce between 
the different modes. This is in it's second incarnation, and I've got some 
ideas to improve it further. I plan on integrating the music manager further
as well as more utility functions.

Did I mention it's rather messy right now? Ugh. I'm working on cleaning it up.





Game Controller:

Navigate Choices: D-Pad or Thumbsticks
Select Choice: Start or A button
Boot to Dash: Press both Thumbsticks
About MXM: Y Button
Game Help: Black Button
Help: White Button
Thumbnail: X Button


IR Controller:

Navigate Choices: Cursor
Select Choice: Select
Boot to Dash: Title
About MXM: Display
Game Help: Info
Help: Menu
Thumbnail:

Keyboard 

Navigate Choices: Cursor Keys
Select Choice: Enter
Boot to Dash: PrtScr
About MXM: Tab
Game Help: F2
Help: F1
Thumbnail: F12

<Help_01>
<HelpTitle>Help Screen 1</HelpTitle>
<DpadX>Select Item</DpadX>
<DpadY>Select Power</DpadY>
<A>Use Item</A>
<B>Use Force</B>
<LThumbY>Move forward/Back</LThumbY>
</Help_01>





PUBLISHER		GAME
0000 Homebrew	 0000 RemoteX
0000 Homebrew    0002 MedialayerName
0fac Homebrew	 e007 Box Media Player
4143 Acclaim	 0003 Legends Of Wrestling
4143 Acclaim	 0004 Turok Evolution
4143 Acclaim	 0006 Burnout
4143 Acclaim	 0007 Aggressive Inline
4143 Acclaim	 000e BMX XXX
4156 Activision	 0004 Tony Hawks Pro Skater 3
4156 Activision	 0005 Blade II
4156 Activision	 0006 Spider-Man
4156 Activision	 0007 X-Men: Next Dimension
4156 Activision	 0009 Rally Fusion : Race Of Champions
4156 Activision	 000a WRECKLESS
4156 Activision	 000b Mat Hoffman's Pro BMX 2
4156 Activision	 000c WRECKLESS
4156 Activision	 0014 Minority Report
4156 Activision	 0017 Tony Hawk's Pro Skater 4
424d Bam         0001 Chase
424d Bam         0002 Reign Of Fire
4253 Bethesda	 0001 Morrowind
4257 BBC         0001 Robot Wars Extreme Destruction
4343 Capcom      0007 MARVEL VS. CAPCOM2
434d Codemasters 0003 Mike Tyson Heavyweight Boxing
434d Codemasters 0008 Prisoner Of War
4356 Crave	 0001 Kabuki Warriors
4356 Crave	 0002 UFC Tapout
4541 EA		 0001 Madden NFL 2002
4541 EA		 0002 Nascar Thunder 2002
4541 EA		 0003 NBA Live 2002
4541 EA		 0004 SSX Tricky
4541 EA		 000d 007 : Agent Under Fire
4541 EA		 000e Harry Potter CoS
4541 EA		 0011 Cel Damage
4541 EA		 0012 Buffy: The Vampire Slayer
4541 EA		 0013 Simpsons Road Rage
4541 EA		 0015 Pirates : Legend Of Black Kat
4541 EA		 001a Medal Of Honor Frontline
4541 EA		 001b NASCAR Thunder
4541 EA		 0026 007™: NightFire™
4541 EA		 0028 NFS Hot Pursuit 2
4541 EA		 0029 TY the Tasmanian Tiger
4543 Encore	 0001 Circus Maximus
4553 Eidos	 0002 Championship Manager
4553 Eidos	 0006 Blood Omen 2
4553 Eidos	 0009 Hitman2: Silent Assassin
4553 Eidos	 000a TimeSplitters 2
4947 Infogrames	 0001 Test Drive Off-Road: Wide Open
4947 Infogrames	 0005 Transworld Surf
4947 Infogrames	 0006 Loons - The Fight For Fame
4947 Infogrames	 0007 Test Drive
4947 Infogrames	 0009 NASCAR® Heat 2002
4947 Infogrames	 000d TAZ: WANTED
4947 Infogrames	 000f Monopoly Party
4947 Infogrames	 0012 Zapper
4947 Infogrames	 0016 Superman: The Man of Steel
4947 Infogrames	 001b Splashdown
4947 Infogrames	 0024 Unreal Championship
4947 Infogrames	 0026 The Terminator: Dawn of Fate
4947 Infogrames	 0028 Nickelodeon Party Blast
494c CrapJap     0002 Braveknight
494d Imagine     0008 OXM Disc 9
494d Imagine     0009 OXM Disc 10
494d Imagine     000a OXM Disc 11
494d Imagine     000b OXM Disc 12
4b42 Kemco            0002 Egg Mania
4b4e Konami      0002 Airforce Delta Storm
4b4e Konami      0004 SILENT HILL 2
4c41 LucasArts   0001 Star Wars Obi-Wan
4c41 LucasArts   0002 Starfighter SE
4c41 LucasArts   0005 Star Wars Jedi Starfighter
4c41 LucasArts   0009 Jedi Knight II
4d41 Complex     5800 MenuX
4d41 Complex     5801 MenuX
4d4a Majesco     0001 BloodRayne
4d4a Majesco     0002 Totaled
4d4a Majesco     0003 Gun Metal
4d53 Microsoft	 0001 Oddworld
4d53 Microsoft	 0002 Fuzion Frenzy
4d53 Microsoft	 0003 Project Gotham Racing
4d53 Microsoft	 0004 Halo
4d53 Microsoft	 0005 Amped
4d53 Microsoft	 0007 Azurik
4d53 Microsoft	 0008 Nightcaster
4d53 Microsoft	 000f Rallisport Challenge
4d53 Microsoft	 0010 Blood Wake
4d53 Microsoft	 0013 BLiNX: the time sweeper
4d53 Microsoft	 0015 Quantum Redshift
4d53 Microsoft	 0017 MechAssault
4d53 Microsoft	 0023 Sneakers
4d53 Microsoft	 002e MAXIMUM CHASE
4d53 Microsoft	 0030 Jockey's Road
4d53 Microsoft	 0032 Shenmue II
4d57 Midway	 0002 Arctic Thunder
4d57 Midway	 0003 NHL Hitz 20-02
4d57 Midway	 0004 Spy Hunter
4d57 Midway	 0006 Defender
4d57 Midway	 0008 Slugfest
4d57 Midway	 000b NHL Hitz 2003
4d57 Midway	 000c MK Deadly Alliance
4d57 Midway	 000e Gauntlet Dark Legacy
4d57 Midway	 0011 Dr. Muto
4d57 Midway	 0012 Fireblade
4e4d Namco	 0001 Smashing Drive
4e4d Namco	 0005 Dead To Rights
4e4d Namco	 0006 Pac-Man World 2
4e4d Namco	 0007 NamcoMuseum
5345 Sega        0003 segaGT 2002
5345 Sega        0004 CRAZY TAXI 3 High Roller
5345 Sega        0008 House of the Dead 3
5345 Sega        000a Jet Set Radio Future
5345 Sega        000b GUNVALKYRIE
5345 Sega        000f ToeJam & Earl III
5345 Sega        0011 NFL2K3
5443 Tecmo	 0001 Dead Or Alive 3
5443 Tecmo	 0004 FATAL FRAME
544d TDK         0001 Shrek
544d TDK         0002 Robotech: Battlecry
544d TDK         0004 Shrek Super Party
5451 THQ         0003 MX 2002 with Ricky Carmichael
5451 THQ         0004 Dark Summit
5451 THQ         0007 Toxic Grind
5451 THQ         0008 MotoGP: Ultimate Racing Technology
5451 THQ         000a Tetris Worlds
5454 God Games   0001 4X4 EVO 2
5454 Rockstar    0003 Max Payne
5454 Gotham      0004 Serious Sam
5553 Ubi Soft    0001 BATMAN VENGEANCE
5553 Ubi Soft    0004 Deathrow
5553 Ubi Soft    0006 Ghost Recon
5655 Interplay	 0003 Crash Bandicoot
5655 Interplay	 0004 The Fellowship of the Ring
5655 Interplay	 0007 Hunter: The Reckoning
5655 Interplay	 000d The Thing
5655 Interplay	 0010 OUTLAW GOLF
5655 Interplay	 0016 Bruce Lee: QOD
5655 Interplay	 0017 Seablade
7867 GotMod      656e neXgen Dashboard
abba Port        fff0 Quake 2X
f000 Evo-X       002d Attribute Fixer v0.2
ffad TeamXecutor f32f DVD-X2 DongleFree DVD-Player
ffff Development 0000 Blinx the time sweeper
ffff Development 051f Boxplorer
ffff Development 7701 Handy Lynx Emulator



   
      
110 Restart marker reply.
120 Service ready in nnn minutes.
125 Data connection already open; transfer starting.
150 File status okay; about to open data connection.

200 Command okay.
202 Command not implemented, superfluous at this site.
211 System status, or system help reply.
212 Directory status.
213 File status.
214 Help message.
215 NAME system type.
220 Service ready for new user.
221 Service closing control connection.
225 Data connection open; no transfer in progress.
226 Closing data connection.
227 Entering Passive Mode (h1,h2,h3,h4,p1,p2).
230 User logged in, proceed.
250 Requested file action okay, completed.
257 "PATHNAME" created.

331 User name okay, need password.
332 Need account for login.
350 Requested file action pending further information.

421 Service not available, closing control connection.
425 Can't open data connection.
426 Connection closed; transfer aborted.
450 Requested file action not taken.
451 Requested action aborted: local error in processing.
452 Requested action not taken.

500 Syntax error, command unrecognized.
501 Syntax error in parameters or arguments.
502 Command not implemented.
503 Bad sequence of commands.
504 Command not implemented for that parameter.
530 Not logged in.
532 Need account for storing files.
550 Requested action not taken.
551 Requested action aborted: page type unknown.
552 Requested file action aborted.
553 Requested action not taken.

 

The different modes of operation for Media X Menu:

"Dashboard Mode"
This is when MXM is run, via the BIOS, as a dashboard for the Xbox. Technically, the Xbox runs
a dashboard with nothing set up (See "Launcher Mode" for more details). "Dashboard" mode may be forced
with a setting in your Preferences.

"Launcher Mode"
This is when MXM is run from the hard drive, but as a secondary application. When the Xbox launched
applications, it normally sets up the application's directory as "D:\", and may (this is fuzzy) set up
a "T:\" and "U:\" as well as a temporary drive to hold cached data used during the game's operation.

"DVD-R Mode"
For MXM, this is when it is run from a DVD-R drive. This is special, because MXM is running purely as
a games launcher, and more specifically, was set up by the disc's creator in a specific fashion. Unless
the creator specifies it, many of the normal 'dashboard' features will not be available, nor will many of
the preferences be used to set up operation.

"XDI Mode"
This is MXM running form a DVD-R using the configuration information from the 
hacked M$ demo menu app, known as "MenuX". MXM is capable of emulating MenuX, except
it does not suffer from the glitchy issues MenuX has sometimes launching
application; MenuX sometimes presents the games configured correctly, but refuses to
launch one or two of them... and it seems to be POSITION dependent, no less, in my
own investigations. I could never pin down why MenuX was failing, or what particular
aspect of the configuration was causing the malfunction. This is one of the reasons I
wrote MXM in the first place.

Now you can simply replace the MenuX default.xbe with MXM's default.xbe and immediately
benefit from the ability to use video previews and multimedia skins, without using the
MXM.xml configuration, for MenuX diehards who are too daunted by MXM's rich capabilities.







Feat: <extendeddescription> tag that grabs a game description from say, xbox.com, ign.com or even ebgames for that matter, based on the XBE name nearest match, or configurable in MXM_entry.xml and displays it. 

just kidding, but a boy can dream.

to stay on topic. It would be nice if the rss stuff could also be pulled up, either from the system menu or on it's own button. There's not alot of real estate as it is with 640x480, but I still have a hankerin for all sorts of rss feeds. I'd love to be able to hit the white button for example and have a sub-page/sub-menu come up with the headlines from ArsTechnica, HardOCP, Shacknews, Xbox-Scene, Allxboxskins, NEWs, the list goes on and on.

The rotating through several rss feeds in one menu block is brilliant though, and I'd take that over nothing anyday.  

I'm actually building support in for categories (up to 32), feeds can belong to multiple categories, then the user may decide to select the category of newsfeed they want.

Skin designers will also be able to incorporate their own newsfeeds as well, to display specific channels and items. When I get the scrolling text, I plan on implementing a means to generate a 'ticker-friendly' string with all item titles, for example, strung together. The skin designer could then scroll all the items across in a section, for example.

I also need to add true multi-line support to the strings, with proper word-wrap.


Request: Adjustable audio mode.

Now, I haven't gone in to see how the current setup is.. but from what I recall, when the background music loops, it fades first before restarting. While this is good for multiple soundtracks, it isn't good for continously looping a single beat. With seamless looping music, much like how video files loop in MXM, you can create a small loop with small filesize but have a seamless soundtrack in the background, be it music or atmospheric. Currently, the main culprit of large skin files is the audio that is included. A feature like this, combined with the random tag, could push a skinner to be creative with background audio without fearing large file sizes (e.g. Generator was a whopping 24MB). 

Solution:
A boolean element (child of Music) to distinguish between fade and seamless audio transitions (i.e. <Music> :: <UseSeamlessAudio>). 




Actually, when the calibration is added, there will be a 'virtual' screen resolution, and it shouldn't be too hard to impliment virtual scaling.

i.e. the skin says it's calibrated, for example, 1080i, and MXM resizes the video to match, if possible.

It's a big change, probably one or two releases past the next one.


2) Second, would it be possible to include a menu editor integrated into the dashboard? No other dash has this, and i think this would provide a good chance to get ahead in the game.

ONce again great work. Im probably asking for a lot. 




Hmmmm... had an epiphany this morning! I'm going to integrate the file manager into the menuing system!!

What does this mean?

When you launch the "File Manager" - it will place the menuing system into a special mode, where you see all the files in a given folder, and drives and directories appear as folders.

You'll see "Media" files in the preview window. 

Select a file and you can mark/unmark it, then go into a system menu to cut/copy/paste/(rename)

Media files and XBEs will be directly executable.

I'll add a conditional so skin designers can fine-tune their menu screens with different features for File Manager Mode.

Sound good? 




I just thought of another feature...

Can you setup an ftp server with the video files in a universal heirachy, and give mxm the ability to download movies by hitting a button? This would also make the dash better imho 



I posted this in my skin thread already.. But I'll do it here as well 

A suggestion for the next release of MXM (though it may be in this one).. It would be cool if we could offset the position of each menu item, like:

<offset>
<item0 xoffset="0" yoffset="0" />
<item1 xoffset="2" yoffset="0" />
<item2 xoffset="-5" yoffset="0" />
</offset>

Which would offset the item from the 'L' and 'T' values here:
<Pos l="375" t="175" w="180" h="175" />

Something like that would make my skin's menu fit a little better



* Master Skins directory setting in Preferences (actually done in my WIP)

* Master Thumbnail and Media directories to pull images/movies out of. How will it work? Image will be named with one of the following methods:

- tn0x00000000.* where the base name is the hexadecimal representation of the game's XBE ID

- tn99999.* where the base name is the decimal representation of the game's XBE ID

- tn(xxx).* where 'xxx' is the XBE embedded name with underscores replacing any spaces.

'tn' prefix is for Thumbnails, 'pv' for preview media. Of course, the extension can be bmp, png, tga, jpg or wmv.

If there is no specific media or thumbnail entry for an item, MXM should access the appropriate master directory and use the image there, if it's found.

This will let you put a large chunk of your media in one place. 



BTW: One of the things I've been thinking about, which should be quite easy, is to automatically tell the Xbox Soundtrack API that any WMA files uploaded to temp drive "X:" (enabled in next release) are to be added to the soundtrack.

This will be akin to the Windows "Font" folder, that does something special when you add files to it.

I also am adding screenshot capability finally.  It will save consecutive screenshots to "Z:"

I don't expect anybody to really want to save much to the "temp" drives, which is why I didn't add them until now. I also need FTP access so I can see changes when I test formatting functionality in the future here. 

Feat: <extendeddescription> tag that grabs a game description from say, xbox.com, ign.com or even ebgames for that matter, based on the XBE name nearest match, or configurable in MXM_entry.xml and displays it. 

just kidding, but a boy can dream.

to stay on topic. It would be nice if the rss stuff could also be pulled up, either from the system menu or on it's own button. There's not alot of real estate as it is with 640x480, but I still have a hankerin for all sorts of rss feeds. I'd love to be able to hit the white button for example and have a sub-page/sub-menu come up with the headlines from ArsTechnica, HardOCP, Shacknews, Xbox-Scene, Allxboxskins, NEWs, the list goes on and on.

The rotating through several rss feeds in one menu block is brilliant though, and I'd take that over nothing anyday. 



Sorting:

<BeginSort>title</BeginSort>

<EndSort>title</EndSort>

Only one sort operation can be active at a time.

SubMenus inherit sorting operations, unless <EndSort /> tag is specified.


Here is my little wish for MXM:

Features:

Support for xFER
Support for SimpleX ISO
Support for XISO
Support for XBE Renamer
Support for Soundtrack editor
...hell....support for any software that works on EVOX.

It would also be cool if MXM had streaming capability built in like XBMP via Relax. This way, the video previews and background music could be streamed from the PC instead of residing on the Xbox HD. 

This is actually a really good idea. Ability to stream the preview vids via SMB or Relax would save some valuable hard drive space. I don't think that it would be very difficult to implement this. Would like to hear BJ's thoughts on this.

I don't think that the current public build will allow you to use Relax. But it's worth a shot.

Well, the preview videos shouldn't really take up too much space, after all, what is another 3 or 4MB in a 1 or 2GB game? Typically the previews are only a fraction of a percent of the total game's size.

Streaming would present several difficulties for the users, actually, if used as previews. What happens when the system isn't on? There would have to be a fallback. It would also take a bit longer to display, as you'd want to buffer the video before you begin displaying.

I could probably handle the streaming stuff, I just don't know how practical it is for the application (I don't really want to build in the functionality of XBMP).

It might be handy, however, to pull updated information (help screens, previews, information) from the local PC... 

streaming media... i was thinkin about this myself also.. but morer on the lines with music. i dont want to clutter my xbox hdd with the same music that i alread y have on my comp.

how about a way to stream music from a internet radio type place? that would be pretty cool too. as it stands now, i have about 3 mp3s in my music folder on the xbox as backgroundd music... but it really sucks hearing the same tunes all the time.


is streaming from internet radio possible? kinda like in xbmp. 

That request is from someone else.. It goes like this, you make a 'random.xml' with skin names in it:

<Skintotal>3</Skintotal>
<Skin>Devoid<Skin>
<Skin>Super Mario Bros</Skin>
<Skin>Simplicity</Skin>

Or something alone those lines, and it randomises the skin based on only those 3 skins, despite how many you might have installed



Here's my take on this new "copy protection". I've been examining these "cracked" default.xbe files and comparing them to the originals, and the only difference seems to be one byte (aside from Riot arbitrarily placing the string "FUCK" in the file), which doesn't seem like much as far as copy-protection circumvention goes. However, I'm now starting to think that developers have moved the media flag.
Every XBE has a media flag, which determines which media on which it can be run. The following is taken from the XDK:

XBEIMAGE_MEDIA_TYPE_HARD_DISK 0x00000001
XBEIMAGE_MEDIA_TYPE_DVD_X2 0x00000002
XBEIMAGE_MEDIA_TYPE_DVD_CD 0x00000004
XBEIMAGE_MEDIA_TYPE_CD 0x00000008
XBEIMAGE_MEDIA_TYPE_DVD_5_RO 0x00000010
XBEIMAGE_MEDIA_TYPE_DVD_9_RO 0x00000020
XBEIMAGE_MEDIA_TYPE_DVD_5_RW 0x00000040
XBEIMAGE_MEDIA_TYPE_DVD_9_RW 0x00000080 

Note that Xbox game discs are a different type of media and have a different flag. At retail, MS signs all Xbox games to run from Xbox game disc only. Not only does the mod chip allow the Xbox to run unsigned code, but it also circumvents this media flag - but only because it knows where the media flag is. I'm theorizing that after moving the media flag, the mod doesn't know how to circumvent it.

I think these "cracks" are a simple hexedit that any one of us could do manually. I'm guessing the hexedit is even simpler from the old 02 00 00 -> FF FF FF job to make the game run on Enigmah Betas - instead of changing 3 bytes we're changing 1. I'm guessing that the media flag will be in the same location in all the new XBEs, so it should be easy to make a universal patcher if that's the case. I'll post more on the topic.

-------------------------------------------------------------------------------------------------------------------

XDXDXD

The long thread was confusing people, so here are instructions for patching - put simply.

All new games are confirmed to have the media check in them. By "new" I mean those games signed by MS after February 2003 or so. If they were signed earlier and mass-produced later, the check will not show up.

If you are running an X2 BIOS 4975 or above, you do not need to be concerned about patching games. This is intended only for those running older BIOSes or mod chips that cannot be flash updated.

You can download the automatic patching tools from the following links:
XBE/ISO Media Patcher v2.0 by Copyhaunters
XBEMIDP2 by L!M!T
ADR Patcher 0.5 Xbox Media Check Fix by ADR-UK
Craxtion v2.1 by LepPpeR

NOTE: XBEMIDP2, Craxtion and ADR Patcher 0.5 Xbox Media Check Fix* contain updated code, so they can patch newer games, including Return to Castle Wolfenstein. XBE/ISO Media Patcher v2.0 can patch games with release dates of Feb 2003 - May 2003.

All the listed utilities can patch both XBE files and ISO files. While ISO patching expectedly takes longer, it is recommended. Patching ISOs ensure that every file in the game is inspected for a media check and if one is found, it is nullified accordingly.

Also, you may prefer to manually patch the file. This is obviously more flexible as you can adjust what string to search for. This can be done with a standard hexediting utility. Simply search the file (either XBE or ISO) for the hex string:
74 4B E8 CA FD FF FF 85 C0 7D 06 33 C0 50 50 EB 44 F6 05
and replace it with
74 4B E8 CA FD FF FF 85 C0 EB 06 33 C0 50 50 EB 44 F6 05
effective changing that blue 7D byte to EB.

Some newer games are showing shorter strings. You may also want to try replacing
E8 CA FD FF FF 85 C0 7D
with
E8 CA FD FF FF 85 C0 EB
Because this is a much shorter string, it is not yet confirmed whether a search for this string will yield false positives and negatively affect the game. Apply this hexedit with caution. *To use this checking routine with ADR's tool, download an updated patch.data file here and overwrite the one in the same folder as ADR's tool.

Although this thread is meant to support others in patching games, it is here so you will read it. Do not ask questions that have already been answered previously in the thread. Also, do not ask where to download the patching tools (linked above), hex editors, or any BIOS image. Due to the excessive length of this thread, any new post that does not abide by the following mandates will be deleted. I will not reply to it, thereby increasing the length of the thread, and I will not ignore it, which would confuse new readers. It will simply be deleted.

Thanks to Xbox-Saves.com for hosting ADR's Patcher and the updated patch.data file. 

This post has been edited by HSDEMONZ on Jul 22 2003, 09:26 AM 



I think I found the catch here. The media flags definitely appear to have moved. I examined both High Heat Baseball 2004 and All-Star Baseball 2004 and there is a clear consistency here. Though the byte offset is not always in the same location, it is easily identifiable. Both original XBE files contain the following hex string:
74 4B E8 CA FD FF FF 85 C0 7D 06 33 C0 50 50 EB 44 F6 05

In the modified XBE files, the same byte changed. The strings were changed to 
74 4B E8 CA FD FF FF 85 C0 EB 06 33 C0 50 50 EB 44 F6 05

As a funny sidenote, Riot's FUCK string replaced bytes 214-217, which WAM had changed to FF FF FF FF. This, of course, was the location of the old media flag.

I'm quite certain this will work on all newer games. Anyone good at programming feel like making an XBE patcher for newer games? LepPpeR?? Any other takers? 
