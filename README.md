<div class="mandoc">
<div class="section">
<h1 id="x4e414d45">NAME</h1> <b class="name">simplestroke</b> &#8212; <span class="desc">a gesture recognition application</span></div>
<div class="section">
<h1 id="x53594e4f50534953">SYNOPSIS</h1><table class="synopsis">
<col style="width: 12.00ex;"/>
<col/>
<tbody>
<tr>
<td>
simplestroke</td>
<td>
<b class="flag">-h</b></td>
</tr>
</tbody>
</table>
<br/>
<table class="synopsis">
<col style="width: 12.00ex;"/>
<col/>
<tbody>
<tr>
<td>
simplestroke</td>
<td>
<b class="flag">new</b> <b class="flag">-d</b> <i class="arg">description</i> <b class="flag">-c</b> <i class="arg">command</i> [<span class="opt"><b class="flag">-w</b> <i class="arg">seconds</i></span>] [<span class="opt"><b class="flag">-h</b></span>]</td>
</tr>
</tbody>
</table>
<br/>
<table class="synopsis">
<col style="width: 12.00ex;"/>
<col/>
<tbody>
<tr>
<td>
simplestroke</td>
<td>
<b class="flag">delete</b> <b class="flag">-i</b> <i class="arg">id</i> [<span class="opt"><b class="flag">-h</b></span>]</td>
</tr>
</tbody>
</table>
<br/>
<table class="synopsis">
<col style="width: 12.00ex;"/>
<col/>
<tbody>
<tr>
<td>
simplestroke</td>
<td>
<b class="flag">detect</b> [<span class="opt"><b class="flag">-hn</b></span>]</td>
</tr>
</tbody>
</table>
<br/>
<table class="synopsis">
<col style="width: 12.00ex;"/>
<col/>
<tbody>
<tr>
<td>
simplestroke</td>
<td>
<b class="flag">export</b> <b class="flag">-i</b> <i class="arg">id</i> [<span class="opt"><b class="flag">-f</b> <i class="arg">file</i></span>] [<span class="opt"><b class="flag">-c</b> <i class="arg">color</i></span>] [<span class="opt"><b class="flag">-h</b></span>]</td>
</tr>
</tbody>
</table>
<br/>
<table class="synopsis">
<col style="width: 12.00ex;"/>
<col/>
<tbody>
<tr>
<td>
simplestroke</td>
<td>
<b class="flag">list</b> [<span class="opt"><b class="flag">-h</b></span>]</td>
</tr>
</tbody>
</table>
</div>
<div class="section">
<h1 id="x4445534352495054494f4e">DESCRIPTION</h1> <b class="name">simplestroke</b> is a gesture detection application.  It is backed by a SQLite database that is used to store the gestures.  Most of the interface consists of manipulating this database.  There are several modes and the first argument determines the mode <b class="name">simplestroke</b> starts in.<dl style="margin-top: 0.00em;margin-bottom: 0.00em;" class="list list-tag">
<dt class="list-tag" style="margin-top: 1.00em;">
<b class="flag">new</b></dt>
<dd class="list-tag" style="margin-left: 10.00ex;">
In this mode <b class="name">simplestroke</b> records a gesture and creates a new entry in the database. Every gesture needs to have a human readable <i class="arg">description</i> and an action or <i class="arg">command</i> to execute on detection of a gesture. The <b class="flag">-c</b> and <b class="flag">-d</b> options are required and  <b>not</b> optional. <b class="name">simplestroke</b> waits for 2 seconds before recording the gesture.  When you are finished drawing the gesture click or press any mouse button or key.<dl style="margin-top: 0.00em;margin-bottom: 0.00em;" class="list list-tag">
<dt class="list-tag" style="margin-top: 1.00em;">
<b class="flag">-c</b> <i class="arg">command</i></dt>
<dd class="list-tag" style="margin-left: 14.00ex;">
The command to execute on detection of the gesture. <i class="arg">command</i> is passed to <i class="file">/bin/sh</i>.</dd>
<dt class="list-tag" style="margin-top: 1.00em;">
<b class="flag">-d</b> <i class="arg">description</i></dt>
<dd class="list-tag" style="margin-left: 14.00ex;">
A human-readable description for the gesture.  The description can be arbitrarily long.</dd>
<dt class="list-tag" style="margin-top: 1.00em;">
<b class="flag">-w</b> <i class="arg">seconds</i></dt>
<dd class="list-tag" style="margin-left: 14.00ex;">
By default <b class="name">simplestroke</b> waits for 2 seconds before recording a gesture. You can override this behavior with <b class="flag">-w</b>. If <i class="arg">seconds</i> is 0, <b class="name">simplestroke</b> will start recording your mouse movements immediately.</dd>
</dl>
</dd>
<dt class="list-tag" style="margin-top: 1.00em;">
<b class="flag">delete</b></dt>
<dd class="list-tag" style="margin-left: 10.00ex;">
This mode removes the given gesture from the database.<dl style="margin-top: 0.00em;margin-bottom: 0.00em;" class="list list-tag">
<dt class="list-tag" style="margin-top: 1.00em;">
<b class="flag">-i</b> <i class="arg">id</i></dt>
<dd class="list-tag" style="margin-left: 14.00ex;">
The <i class="arg">id</i> of the gesture to delete.</dd>
</dl>
</dd>
<dt class="list-tag" style="margin-top: 1.00em;">
<b class="flag">detect</b></dt>
<dd class="list-tag" style="margin-left: 10.00ex;">
This mode starts gesture detection.  Draw your gesture and then confirm by clicking any mouse button.<div class="spacer">
</div>
More specifically, <b class="name">simplestroke</b> reacts on the release of any mouse button.  This makes it possible to start <b class="name">simplestroke</b> with a pressed mouse button and start gesture analysis after releasing it.<dl style="margin-top: 0.00em;margin-bottom: 0.00em;" class="list list-tag">
<dt class="list-tag" style="margin-top: 1.00em;">
<b class="flag">-n</b></dt>
<dd class="list-tag" style="margin-left: 14.00ex;">
Do not execute the command associated with a detected gesture, but print information about it instead, if any.</dd>
</dl>
</dd>
<dt class="list-tag" style="margin-top: 1.00em;">
<b class="flag">export</b></dt>
<dd class="list-tag" style="margin-left: 10.00ex;">
Outputs the gesture with the id given by <b class="flag">-i</b> as an SVG.  Output goes to <i class="arg">stdout</i>. Use  <b>display</b> from the ImageMagick package to view it. Alternatively, any modern browser will render SVGs.<dl style="margin-top: 0.00em;margin-bottom: 0.00em;" class="list list-tag">
<dt class="list-tag" style="margin-top: 1.00em;">
<b class="flag">-i</b> <i class="arg">id</i></dt>
<dd class="list-tag" style="margin-left: 14.00ex;">
The id of the gesture to export.</dd>
<dt class="list-tag" style="margin-top: 1.00em;">
<b class="flag">-c</b> <i class="arg">color</i></dt>
<dd class="list-tag" style="margin-left: 14.00ex;">
The stroke color, can be in any format that is supported by SVG.</dd>
</dl>
</dd>
<dt class="list-tag" style="margin-top: 1.00em;">
<b class="flag">list</b></dt>
<dd class="list-tag" style="margin-left: 10.00ex;">
Lists all gestures.  Output goes to <i class="arg">stdout</i>.</dd>
</dl>
</div>
<div class="section">
<h1 id="x4558414d504c4553">EXAMPLES</h1><div class="subsection">
<h2 id="x5354415254494e4720444554454354494f4e">STARTING DETECTION</h2> To start gesture detection in every day situations, you might want to setup your desktop environment to start  <b>simplestroke detect</b>.<div class="spacer">
</div>
E.g. to start <b class="name">simplestroke</b> with  <b>xbindkeys</b> while holding down the 9th mouse button add this to your <i class="file">~/.xbindkeysrc</i><div class="spacer">
</div>
<pre style="margin-left: 5.00ex;" class="lit display">
&quot;simplestroke detect&quot; 
        b:9</pre>
<div class="spacer">
</div>
Hold the mouse button and after you are finished drawing your gesture, release it. <b class="name">simplestroke</b> will then analyse your gesture and determine which command to start, if any.<div style="height: 1.00em;">
&#160;</div>
To start gesture detection with your keyboard add this to your <i class="file">~/.xbindkeysrc</i><div class="spacer">
</div>
<pre style="margin-left: 5.00ex;" class="lit display">
&quot;simplestroke detect&quot; 
        Mod4 + c + Release</pre>
<div class="spacer">
</div>
or if you use <b>i3</b> add this to your <i class="file">~/.config/i3/config</i><div class="spacer">
</div>
<pre style="margin-left: 5.00ex;" class="lit display">
bindsym --release $mod+c</pre>
<div class="spacer">
</div>
the release options prevents <b>xbindkeys</b> or <b>i3</b> from spawning multiple <b class="name">simplestroke</b> processes.  Draw your gesture and press a mouse button to start detection.<div class="spacer">
</div>
Note that binding to mouse buttons is currently not possible in <b>i3</b> 4.8.</div>
</div>
<div class="section">
<h1 id="x53454520414c534f">SEE ALSO</h1> <a class="link-man" href="http://www.openbsd.org/cgi-bin/man.cgi/OpenBSD-current/man1/display.1?query=display&amp;sec=1">display(1)</a> <a class="link-man" href="http://www.openbsd.org/cgi-bin/man.cgi/OpenBSD-current/man1/i3.1?query=i3&amp;sec=1">i3(1)</a> <a class="link-man" href="http://www.openbsd.org/cgi-bin/man.cgi/OpenBSD-current/man1/xbindkeys.1?query=xbindkeys&amp;sec=1">xbindkeys(1)</a></div>
<div class="section">
<h1 id="x415554484f5253">AUTHORS</h1> <span class="author">Tobias Kortkamp</span> &lt;<a class="link-mail" href="mailto:t@tobik.me">t@tobik.me</a>&gt;<div class="spacer">
</div>
<b class="name">simplestroke</b> is inspired and based on  <b>easystroke</b> 0.6.0 written by Thomas Jaeger &#10216;<a class="link-ext" href="https://github.com/thjaeger/easystroke">https://github.com/thjaeger/easystroke</a>&#10217;.</div>
</div>

