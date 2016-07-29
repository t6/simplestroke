<div class="mandoc">
<table summary="Document Header" class="head" width="100%">
<col width="30%">
<col width="30%">
<col width="30%">
<tbody>
<tr>
<td class="head-ltitle">
SIMPLESTROKE(1)</td>
<td class="head-vol" align="center">
FreeBSD General Commands Manual</td>
<td class="head-rtitle" align="right">
SIMPLESTROKE(1)</td>
</tr>
</tbody>
</table>
<div class="section">
<h1 id="x4e414d45">NAME</h1> <b class="name">simplestroke</b> &#8212; <span class="desc">a small tool for detecting mouse gestures</span></div>
<div class="section">
<h1 id="x53594e4f50534953">SYNOPSIS</h1><table class="synopsis">
<col style="width: 12.00ex;">
<col>
<tbody>
<tr>
<td>
simplestroke</td>
<td>
&#91;<span class="opt"><b class="flag">&#45;s</b></span>&#93;</td>
</tr>
</tbody>
</table>
</div>
<div class="section">
<h1 id="x4445534352495054494f4e">DESCRIPTION</h1> <b class="name">simplestroke</b> is a simple application for detecting mouse gestures.  There are ten pre-defined mouse gesture you can choose from.<p>
With no arguments <b class="name">simplestroke</b> starts in gesture detection mode.  Draw your gesture and then confirm by clicking any mouse button.<p>
More specifically, <b class="name">simplestroke</b> reacts on the release of any mouse button.  This makes it possible to start <b class="name">simplestroke</b> with a pressed mouse button and start gesture analysis after releasing it.<p>
<b class="name">simplestroke</b> prints the name of the detected gesture, if any.  The output can then be used in a simple shell script to execute commands.  Use <b class="flag">&#45;s</b> to print a small script template you can base your script on.</div>
<div class="section">
<h1 id="x4745535455524553">GESTURES</h1> The following gestures are supported.  The names are derived from the direction you would draw them in.<div class="subsection">
<h2 id="x5354524149474854204c494e45204745535455524553">STRAIGHT LINE GESTURES</h2><pre style="margin-left: 0.00ex;" class="lit display">
TopDown 
DownTop 
LeftRight 
RightLeft</pre>
</div>
<div class="subsection">
<h2 id="x444941474f4e414c204745535455524553">DIAGONAL GESTURES</h2><pre style="margin-left: 0.00ex;" class="lit display">
TopLeftDown 
TopRightDown 
DownLeftTop 
DownRightTop</pre>
</div>
<div class="subsection">
<h2 id="x5a204745535455524553">Z GESTURES</h2><pre style="margin-left: 0.00ex;" class="lit display">
LeftZ 
RightZ</pre>
</div>
<div class="subsection">
<h2 id="x535155415245204745535455524553">SQUARE GESTURES</h2><pre style="margin-left: 0.00ex;" class="lit display">
SquareLeft 
SquareRight</pre>
</div>
</div>
<div class="section">
<h1 id="x4558414d504c4553">EXAMPLES</h1> The following examples assume that  <b>simplestroke.sh</b> is the shell script you call <b class="name">simplestroke</b> from.<div class="subsection">
<h2 id="x5354415254494e4720444554454354494f4e">STARTING DETECTION</h2> To start detection with  <b>xbindkeys</b> while holding down the 9th mouse button add this to your <i class="file">~/.xbindkeysrc</i><p>
<pre style="margin-left: 5.00ex;" class="lit display">
"simplestroke.sh" 
        b:9</pre>
<p>
Hold the mouse button and after you are finished drawing your gesture, release it.<p>
To start gesture detection with your keyboard add this to your <i class="file">~/.xbindkeysrc</i><p>
<pre style="margin-left: 5.00ex;" class="lit display">
"simplestroke.sh" 
        Mod4 + c + Release</pre>
<p>
or if you use <b>i3</b> add this to your <i class="file">~/.config/i3/config</i><p>
<pre style="margin-left: 5.00ex;" class="lit display">
bindsym --release $mod+c exec simplestroke.sh</pre>
<p>
the release options prevents <b>xbindkeys</b> or <b>i3</b> from spawning multiple <b class="name">simplestroke</b> processes.</div>
</div>
<div class="section">
<h1 id="x53454520414c534f">SEE ALSO</h1> <a class="link-man" href="https://www.freebsd.org/cgi/man.cgi?query=i3&amp;sektion=1&amp;apropos=0&amp;manpath=FreeBSD+10.3-RELEASE+and+Ports">i3(1)</a> <a class="link-man" href="https://www.freebsd.org/cgi/man.cgi?query=xbindkeys&amp;sektion=1&amp;apropos=0&amp;manpath=FreeBSD+10.3-RELEASE+and+Ports">xbindkeys(1)</a></div>
<div class="section">
<h1 id="x415554484f5253">AUTHORS</h1> <span class="author">Tobias Kortkamp</span> &#60;<a class="link-mail" href="mailto:t@tobik.me">t@tobik.me</a>&#62;<p>
<b class="name">simplestroke</b> is inspired and based on  <b>easystroke</b> 0.6.0 written by Thomas Jaeger &#60;<a class="link-ext" href="https://github.com/thjaeger/easystroke">https://github.com/thjaeger/easystroke</a>&#62;.</div>
<table summary="Document Footer" class="foot" width="100%">
<col width="50%">
<col width="50%">
<tbody>
<tr>
<td class="foot-date">
July 28, 2016</td>
<td class="foot-os" align="right">
FreeBSD 10.3</td>
</tr>
</tbody>
</table>
</div>

