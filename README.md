<div class="mandoc">
<table summary="Document Header" class="head" width="100%">
<col width="30%">
<col width="30%">
<col width="30%">
<tbody>
<tr>
<td class="head-ltitle">
SIMPLESTROKE-DAEMON(1)</td>
<td class="head-vol" align="center">
FreeBSD General Commands Manual</td>
<td class="head-rtitle" align="right">
SIMPLESTROKE-DAEMON(1)</td>
</tr>
</tbody>
</table>
<div class="section">
<h1 id="x4e414d45">NAME</h1> <b class="name">simplestroke</b> &#8212; <span class="desc">detect mouse gestures</span></div>
<div class="section">
<h1 id="x53594e4f50534953">SYNOPSIS</h1><table class="synopsis">
<col style="width: 12.00ex;">
<col>
<tbody>
<tr>
<td>
simplestroke</td>
<td>
</td>
</tr>
</tbody>
</table>
</div>
<div class="section">
<h1 id="x4445534352495054494f4e">DESCRIPTION</h1> <b class="name">simplestroke</b> detects mouse gestures.  There are twelve pre-defined mouse gestures you can choose from.<p>
With no arguments <b class="name">simplestroke</b> starts in gesture detection mode.  Draw your gesture and then confirm by clicking any mouse button.<p>
<b class="name">simplestroke</b> reacts on the release of mouse buttons.  This makes it possible to start <b class="name">simplestroke</b> from your window manager while pressing down a mouse button and start gesture analysis after releasing it.<p>
<b class="name">simplestroke</b> prints the name of the detected gesture, if any.  The output can then be used in a simple shell script to execute commands.</div>
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
<h1 id="x4558414d504c4553">EXAMPLES</h1> The following examples assume that <i class="file">simplestroke.sh</i> is the shell script you call <b class="name">simplestroke</b> from:<p>
<pre style="margin-left: 5.00ex;" class="lit display">
#!/bin/sh 
case $(simplestroke) in 
    TopDown) 
    ;; 
    DownTop) 
    ;; 
    LeftRight) 
    ;; 
    RightLeft) 
    ;; 
    TopLeftDown) 
    ;; 
    TopRightDown) 
    ;; 
    DownLeftTop) 
    ;; 
    DownRightTop) 
    ;; 
    LeftZ) 
    ;; 
    RightZ) 
    ;; 
    SquareLeft) 
    ;; 
    SquareRight) 
    ;; 
    *) 
        exit 1 
esac</pre>
<p>
To start detection with <a class="link-man" href="https://man.freebsd.org/simplestroke-daemon(1)">simplestroke-daemon(1)</a> while holding down the 9th mouse button add this to your <i class="file">~/.xinitrc</i>:<p>
<pre style="margin-left: 2.00ex;" class="lit display">
<b class="cmd">simplestroke-daemon -c simplestroke.sh -b 9</b></pre>
<p>
Hold the mouse button and after you are finished drawing your gesture, release it.</div>
<div class="section">
<h1 id="x53454520414c534f">SEE ALSO</h1> <a class="link-man" href="https://man.freebsd.org/simplestroke-daemon(1)">simplestroke-daemon(1)</a></div>
<div class="section">
<h1 id="x415554484f5253">AUTHORS</h1> <span class="author">Tobias Kortkamp</span> &#60;<a class="link-mail" href="mailto:t@tobik.me">t@tobik.me</a>&#62;<p>
<b class="name">simplestroke</b> is inspired and based on  <b>easystroke</b> 0.6.0 written by Thomas Jaeger &#60;<a class="link-ext" href="https://github.com/thjaeger/easystroke">https://github.com/thjaeger/easystroke</a>&#62;.</div>
<table summary="Document Footer" class="foot" width="100%">
<col width="50%">
<col width="50%">
<tbody>
<tr>
<td class="foot-date">
September 2, 2016</td>
<td class="foot-os" align="right">
FreeBSD 10.3</td>
</tr>
</tbody>
</table>
</div>

