<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN"
"http://www.w3.org/TR/REC-html40/loose.dtd">
<!-- $Id: zimpl.php,v 1.5 2001/10/30 14:56:18 thor Exp $ -->
<?php
#
#	Author: Philip Gatt
#	Email: pxg@hotmail.com
#	Date: 6-22-99
#	Location: Orlando, Florida
#
#	This file is free to use, modify, distribute, delete, distort, 
#       or whatever else you desire.
#	I only ask that I receive credit and that you send me an email 
#       letting me know you are using my script (so I know I helped somebody).
#
#	modified by Thorsten Koch (koch@zib.de) 11Mar2001
#
$counter = "counter/counter.dat";  # the data storage file

if (file_exists($counter)) {
	$fp = fopen($counter, 'r');
	$buffer = fread($fp, filesize($counter));
	fclose($fp);
	$visitorCount = intval($buffer);
}
else
{
	$visitorCount = 0;
}
$fp = fopen($counter, 'w');
fwrite($fp, ++$visitorCount);
fclose($fp);
?>
<html lang="en">
<head>
<meta name="author" content="Thorsten Koch">
<meta name="description" content="Zimple Homepage">
<meta name="keywords" content="ZIMPL, Mathematical Modelling Language, Mathematical Programming, Optimization, Algebraic Modelling Language">
<meta name="date" content="$Date: 2001/10/30 14:56:18 $">
<meta name="robots" content="index|follow">
<meta http-equiv="content-type" content="text/html;charset=iso-8859-1">
<title>Zimpl</title> 
</head>
<body bgcolor="#FFFFFF" text="#000000" 
      link="#0000FF" alink="#FF0000" vlink="#800080">
<p>
<!-- <img src="http://www.zib.de/global/images/zib_logo1.gif" -->

<a href="http://www.zib.de/index.en.html">
   <img src="images/zib_logo1.gif" 
   alt="Goto ZIB" border=0 align=right width=86 height=118>
</a>
</p>  

<h1>Zimpl</h1>
<h3>What is Zimpl</h3>
<p>
Zimpl is a little language to translate the mathematical model of a 
problem into a linear or (mixed-) integer mathematical program
expressed in .lp or .mps file format which can be read
by a LP or MIP solver.
</p>
<h3>Where does is run?</h3>
<p>
Zimpl is a command line 
program written in plain C and released under GNU GPL. 
It has been tested to compile under
Linux/Intel, Linux/Alpha, Solaris and Tru64. 
Probably it will compile and run werever a decent OS and C compiler
is available (I heard this includes AIX). There is even some
documentation. There is <em>no</em> Windows version.</p>
<p>
<hr noshade>
<h3>Download</h3>
<p>
The latest Version is 1.01. Download complete source code and documentation
<a href="zimpl-1.01.tgz">here</a>. 
<br>
Differences to the older versions are in the <a href="CHANGELOG">Changelog</a>.
</p>
<h3>Bugs</h3>
<p>
If you find one (shouldn't be to difficult), it would be nice if you 
send a description 
or even better a working fix with a runnable example .zpl file to 
<a href="mailto://koch@zib.de">me.</a>
</p>
<hr noshade>
<h3>Links</h3>
<h4>The commercial ones:</h4>
<p>
<a href="http://www.ampl.com">AMPL</a>,
<a href="http://www.ilog.com">ILOG</a>,
<a href="http://www.gams.com">GAMS</a>,
<a href="http://www.dash.co.uk">XPRESS-MP</a>,
<a href="http://www6.software.ibm.com/es/oslv2/features/welcome.htm">OSL2</a>.
<a href="http://www.paragon.nl">AIMMS</a>,
<a href="http://www.lindo.com">LINDO</a>,
</p>
<h4>Interesting sites</h4>
<dl>
<dt><a href="http://plato.la.asu.edu/guide.html">
    Decision Tree for Optimization Software</a></dt>
<dt><a href="http://elib.zib.de/">
    The electronical library for mathematical software (eLib)</a></dt>
<dt><a href="http://optnet.itwm.uni-kl.de/opt-net/">
    Opt-Net</a></dt>
<dt><a href="http://www.cudenver.edu/~hgreenbe/glossary/glossary.html">
    Mathematical Programming GlossaryŠ by Harvey J. Greenberg</a></dt>
<dt><a href="http://mat.gsia.cmu.edu">
   Michael Trick's Operations Research Page</a></dt>
<dt><a href="http://www-neos.mcs.anl.gov">
   NEOS Server for Optimization</a></dt>
</dl>
<p>
<hr noshade>
<a href="http://www.anybrowser.org/campaign/">
<img src="images/anybrowser3.gif" alt="Best viewed with any browser" 
border="0" height="31" width="88" align="bottom"></a>
<a href="http://www.gimp.org">
<img src="images/gfx_by_gimp.gif" alt="Graphics by GIMP" 
border="0" height="36" width="90" align="bottom"></a> 
<img src="images/msfree.gif" alt="100% Microsoft Free" 
border="0" width="100" height="31" align="bottom">    
<a href="http://www.php.net">
<img src="images/phpsmall.gif" alt="Powered by PHP" 
border="0" height="31" width="88" align="bottom"></a>
<a href="http://petition.eurolinux.org">
<img src="images/patent_button.gif" alt="No ePatents" 
border="0" width="88" height="36" align="bottom"></a>    
<a href="http://www.gnu.org">
<img src="images/gnuspirit.png" alt="The GNU Project" 
 border="0" width="100" height="36" align="bottom"></a>
<a href="http://validator.w3.org/check/referer">
<img src="images/valid-html40.png" alt="Valid HTML 4.0!"
border="0" height="31" width="88" align="bottom"></a>
<hr noshade><address><font size=-1>
Last Update $Date: 2001/10/30 14:56:18 $ by
<a href="/personal/personal.pl?name=koch">Thorsten Koch</a>
<br>&copy; 2001 by Thorsten Koch
</font>
</address>
</body>
</html>


