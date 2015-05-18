  * build takes a long time since refactor of parser
    * install vs2013 express
      * causes incremental linking to fail with: LINK : fatal error LNK1123: failure during conversion to COFF: file invalid or corrupt
        * install vs2010 sp1 + compiler update
    * vs2010sp1 build timings: full rebuild, or incremental build (modifying Sema.cpp, either incremental or full link)
      * Debug/Win32: full: 01:49 (link:00:12, cl:1:37), Sema (incr): 01:04 (link 00:25, cl 00:39), Sema (full): 00:54 (link 00:05, cl 00:48)
      * Debug/Win64: full: 01:43 (link:00:12, cl:1:31), Sema (incr): 02:49 (link 02:00, cl 00:49), Sema (full): 00:54 (link 00:06, cl 00:47)
      * Release/Win32: full: 03:50 (link:02:22, cl:1:27)
    * vs2013 Update 3 build timings: full rebuild, or incremental build (modifying Sema.cpp, either incremental or full link)
      * Debug/Win32: full: 01:07 (link:00:04, cl:1:03), Sema (incr): 00:54 (link 00:28, cl 00:26), Sema (full): 00:29 (link 00:04, cl 00:25)
      * Debug/Win64: full: 01:25 (link:00:15, cl:1:10), Sema (incr): 02:41 (link 02:09, cl 00:32), Sema (full): 00:36 (link 00:04, cl 00:32)
      * /Zc:inline option has no effect on link time
  * port to boost 1.56
    * #define _VARIANT\_BOOL /##/
      * pasting slashes together is non-standard: modify windows platform headers
    * fails parsing boost: requires__is\_convertible\_to and__is\_abstract
      *_MSC\_VER roll back to msvc 7.1
        * failed parsing is\_class\_impl
    * add boost wave changes to svn
  * port to vs2013
    * ifstream::readsome fails: use rdbuf()->sgetn()
    * no STLPort, but release performance appears comparable to before
      * profile!
    * debug performance appears slower by ~10-20%
      * profile shows <5% time idling/spinning
      * disabling output streams improves things by <5%
      * try _ITERATOR\_DEBUG\_LEVEL=0 .. not compatible with boost?
  * Reconcile Minimal Rebuild between x64 and Win32_

