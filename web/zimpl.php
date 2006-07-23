<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" 
"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<!-- $Id: zimpl.php,v 1.37 2006/07/23 19:49:49 bzfkocht Exp $ -->
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
<meta name="author" content="Thorsten Koch" />
<meta name="description" content="Zimpl Homepage" />
<meta name="keywords" content="ZIMPL, Mathematical Modelling Language, Mathematical Programming, Optimization, Algebraic Modelling Language" />
<meta name="date" content="$Date: 2006/07/23 19:49:49 $" />
<meta name="robots" content="index|follow" />
<meta http-equiv="content-type" content="text/html;charset=iso-8859-1" />
<link rel="stylesheet" href="zimpl.css" type="text/css" />
<link rel="shortcut icon" href="zimpl.ico" />
<title>Zimpl</title> 
</head>
<body>
<p>
<a href="http://www.zib.de/index.en.html">
   <img src="images/ziblogo64x100.gif" 
   alt="Goto ZIB" class="logo" width="64" height="100" />
</a>
</p>  
<h1>Zimpl</h1>
<h3>What is Zimpl</h3>
<p>
Zimpl is a little language to translate the mathematical model of a 
problem into a linear or (mixed-) integer mathematical program
expressed in .lp or .mps
file format which can be read and (hopefully) solved by a LP or MIP solver.
</p>
<h3>Where does is run?</h3>
<p>
Zimpl is a command line 
program written in plain C and released under 
<a href="http://www.gnu.org">GNU</a>
<a href="http://www.gnu.org/licenses/gpl.html">GPL</a>. 
It has been tested to compile under
Linux/Intel, Solaris, Tru64, HPUX, IRIX, AIX and MacOS-X. 
Probably it will compile and run wherever 
<a href="http://www.swox.com/gmp/">GMP</a> 
is available. Zimpl has even been successfully compiled for Windows using 
<a href="http://www.mingw.org">MinGW</a> and the 
<a href="http://gcc.gnu.org">GCC</a> as a cross compiler.
</p>
<hr />
<h3>What's new in 2.04</h3>
<ul>
<li>Bug fixes</li>
<li>Unions of indexed sets</li>
<li>Reading of single values</li>
<li>Declaration of SOSs</li>
<li>Now usable as a library.</li>
</ul>
<h3>Download</h3>
<p>
The latest Version is 2.04. 
<a href="download">Download</a>
complete
<a href="download/zimpl-2.04.tgz">source code</a>
and 
<a href="download/zimpl.pdf">documentation</a>.
There is also a <a
href="http://packages.debian.org/cgi-bin/search_packages.pl?keywords=zimpl&searchon=names&version=all&release=all">Debian
package</a> thanks to Joachim Reichel.
<br />
Differences to the older versions are described in the 
<a href="download/CHANGELOG.txt">Changelog</a>.
<br />
Precompiled 
<a href="download">binaries</a> are also available.
</p>
<h3>Bugs</h3>
<p>
If you find one (shouldn't be too difficult), it would be nice if you 
send a description 
or even better a working fix with a runnable example .zpl file to 
koch<img src="images/klammeraffe.png" class="inline" alt="Klammeraffe" />zib.de.
</p>
<hr />
<h3>Solver</h3>
<p>
You need a solver for the problems generated by Zimpl. 
<a href="http://scip.zib.de">SCIP</a> is a solver for mixed integer programs 
which can be linked directly with Zimpl. 
<a href="http://lpsolve.sourceforge.net">lp_solve</a> is another solver which has an
experimental integration of Zimpl.
<a href="http://www.zib.de/Optimization/Software/Soplex">SoPlex</a>
is a good simplex solver for pure linear programs (LP). 
<br />
For other available solvers, see the 
<a href="http://plato.la.asu.edu/topics/problems/nlores.html#LP-problem">
Decision Tree for Optimization Software / LP-Software</a>.
</p>
<hr />
<h3>Links</h3>
<h4>Zimpl related:</h4>
<dl>
<dt><a href="http://www.math.tu-berlin.de/Vorlesungen/WS02/LinOpt/">
    TU Berlin, Lecture on Linear Optimization</a></dt>
<dt><a href="http://www.mathematik.tu-darmstadt.de/ags/ag7/Lehrveranstaltungen/WS2002_de.html">
    TU Darmstadt, Lecture on Discrete Optimization</a></dt>
<dt><a href="http://www.math.uni-augsburg.de/~schmidtb/bschmidt/OptiI03/zimpl.html">
    University Augsburg, Lecture on Optimization</a></dt>
<dt>
<a
href="http://www.uni-bayreuth.de/departments/wirtschaftsmathematik/rambau/Teaching/Uni_Bayreuth/WS_2004/Diskrete_Optimierung_Anwendungen/">University
Bayreuth, Lecture on Diskrete Optimierung in Telekommunication,
Logistic and Transport</a></dt>
<dt><a href="http://www.wm.uni-bayreuth.de/index.php?id=linopt">University
Bayreuth, Lecture on Linear Optimization</a></dt>
<dt><a href="http://co-at-work.zib.de">Matheon, Combinatorial
Optimization at Work</a></dt>
</dl>
<h4>The commercial ones:</h4>
<p>
<a href="http://www.ampl.com">AMPL</a>,
<a href="http://www.ilog.com">ILOG</a>,
<a href="http://www.gams.com">GAMS</a>,
<a href="http://www.dashoptimization.com">XPRESS-MP</a>,
<a href="http://www.aimms.com">AIMMS</a>,
<a href="http://www.lindo.com">LINDO</a>,
</p>
<h4>Interesting sites</h4>
<dl>
<dt><a href="http://plato.la.asu.edu/guide.html">
    Decision Tree for Optimization Software</a></dt>
<dt><a href="http://optnet.itwm.uni-kl.de/opt-net/">
    Opt-Net</a></dt>
<dt><a href="http://www.cudenver.edu/~hgreenbe/glossary/glossary.html">
    Mathematical Programming GlossaryŠ by Harvey J. Greenberg</a></dt>
<dt><a href="http://www.informs.org/Resources/">
   INFORMS OR/MS Resource Collection</a></dt>
<dt><a href="http://www-neos.mcs.anl.gov">
   NEOS Server for Optimization</a></dt>
</dl>
<hr />
<div>
<a href="http://validator.w3.org/check/referer"><img 
 class="logobar" src="images/valid-xhtml11.png"
 alt="Valid XHTML 1.1!" height="31" width="88" /></a>
<a href="http://jigsaw.w3.org/css-validator/check/referer"><img 
 class="logobar" src="images/vcss.png" 
 alt="Valid CSS!" height="31" width="88" /></a>
<a href="http://www.anybrowser.org/campaign/">
<img src="images/anybrowser3.gif" alt="Best viewed with any browser" 
 class="logobar" height="31" width="88" /></a>
<a href="http://www.gimp.org">
<img src="images/gfx_by_gimp.gif" alt="Graphics by GIMP" 
 class="logobar" height="36" width="90" /></a> 
<img src="images/msfree.gif" alt="100% Microsoft Free" 
 class="logobar" width="100" height="31" />    
<a href="http://www.php.net">
<img src="images/phpsmall.gif" alt="Powered by PHP" 
 class="logobar" height="31" width="88" /></a>
<a href="http://petition.eurolinux.org">
<img src="images/patent_button.gif" alt="No ePatents" 
 class="logobar" width="88" height="36" /></a>    
<a href="http://www.gnu.org">
<img src="images/gnuspirit.png" alt="The GNU Project" 
 class="logobar" width="100" height="36" /></a>
</div>
<hr /><address>
Last Update $Date: 2006/07/23 19:49:49 $ by
<a href="/mitarbeiter/mitarbeiter.pl?email=koch&amp;language=en">Thorsten Koch</a>
<br />&copy; 2005 by Thorsten Koch,
<a href="/impressum/index.en.html">Imprint</a>
</address>
</body>
</html>
<?php
define("_BBC_PAGE_NAME", "Zimpl");
define("_BBCLONE_DIR", "../marsupilami/bbclone/");
define("COUNTER", _BBCLONE_DIR."mark_page.php");
if (is_readable(COUNTER)) include_once(COUNTER);
?>
<!-- Emacs Local Variables:     -->
<!-- Emacs mode:html            -->
<!-- Emacs tab-width:8          -->
<!-- Emacs indent-tabs-mode:nil -->
<!-- Emacs End:                 -->


