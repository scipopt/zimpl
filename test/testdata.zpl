# $Id: testdata.zpl,v 1.1 2001/05/06 11:43:21 thor Exp $
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
#*                                                                           * 
#*   File....: testdata.zpl                                                  * 
#*   Name....: ZIMPLE Test Modell Data                                       * 
#*   Author..: Thorsten Koch                                                 * 
#*   Copyright by Author, All rights reserved                                * 
#*                                                                           * 
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
#
#  These are the SET declarations for testall.zpl
# 
set A  := { <1>, <2>, <3> };
set B  := { 4 to 8 };
set C  := { 2 to 10 by 2 };
set D  := { "Hallo", "Tach", "Moin" };
set E  := A union B;
set F  := E inter C;
set DD := D cross { 1, 2 };
set G  := DD without { <2, "Tach">, <1, "Moin"> };
set H  := A symdiff C;
