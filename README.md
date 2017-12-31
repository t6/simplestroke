SIMPLESTROKE-DAEMON(1) - FreeBSD General Commands Manual

# NAME

**simplestroke** - detect mouse gestures

# SYNOPSIS

**simplestroke**

# DESCRIPTION

**simplestroke**
detects mouse gestures.  There are twelve pre-defined mouse gestures
you can choose from.

With no arguments
**simplestroke**
starts in gesture detection mode.  Draw your gesture and then confirm by
clicking any mouse button.

**simplestroke**
reacts on the release of mouse buttons.  This makes it possible to
start
**simplestroke**
from your window manager while pressing down a mouse button and start
gesture analysis after releasing it.

**simplestroke**
prints the name of the detected gesture, if any.  The output can then
be used in a simple shell script to execute commands.

# GESTURES

The following gestures are supported.  The names are derived from the
direction you would draw them in.

## STRAIGHT LINE GESTURES

	TopDown
	DownTop
	LeftRight
	RightLeft

## DIAGONAL GESTURES

	TopLeftDown
	TopRightDown
	DownLeftTop
	DownRightTop

## Z GESTURES

	LeftZ
	RightZ

## SQUARE GESTURES

	SquareLeft
	SquareRight

# EXAMPLES

The following examples assume that
*simplestroke.sh*
is the shell script you call
**simplestroke**
from:

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
	esac

To start detection with
simplestroke-daemon(1)
while holding down the 9th
mouse button add this to your
*~/.xinitrc*:

	**simplestroke-daemon -c simplestroke.sh -b 9**

Hold the mouse button and after you are finished drawing your gesture,
release it.

# SEE ALSO

simplestroke-daemon(1)

# AUTHORS

Tobias Kortkamp &lt;[t@tobik.me](mailto:t@tobik.me)&gt;

**simplestroke**
is inspired and based on **easystroke** 0.6.0 written by Thomas Jaeger
&lt;[https://github.com/thjaeger/easystroke](https://github.com/thjaeger/easystroke)&gt;.

FreeBSD 12.0-CURRENT - September 2, 2016
