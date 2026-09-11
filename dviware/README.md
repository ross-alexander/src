---
title: DVI viewer using GTK4 supporting PK fonts
author: Ross Alexander
date: September 11, 2026
---

# Updates

2026-09-10
 ~ Update to use cairomm-1.16 and gtk+-3
 ~ Still had issues because gtkmm-3 uses cairomm-1.0
 ~ Update to gtk4, gtkmm4, cairomm-1.16
 ~ Mostly using GTK rather than GTKMM, except to wrap cairo_t* to Cairo::RefPtr<Cairo::Context>

# kpathsea issues

TeXLive uses the kpathsea library in many of this executables to find
files.  In particular the main TeX varient, such as pdfTeX, XeTeX and
LuaTex, as well as xdvi and dvips.  The library in turn uses `ls-R`
files rather than actually searching directories.  This is controlled
by `texmf.cnf` files.

When installing TeX by hand from the TeXLive sources the base
installation is set by the `--prefix` option on configure, normally
either `/usr/local/texlive/2026` or `/opt/texlive/2026`, depending on
the year.  The TeXLive installation is three tar balls, one is the
source code, the second the primary `texmf` files and the third extras
for TeXLive utilities.

The `texmf` is unpacked into _prefix_/share/texmf-dist with the
`texmf.cfg` in the `web2c` subdirectory.

While the `prefix` option on `configure` controls the installation
of the executables it does not embed the prefix into libkpathsea.
Instead it uses `$SELFAUTOLOC` and `$SELFAUTODIR`.

~~~
speedy 09:24:58 ~$ kpsewhich -var-value=SELFAUTOLOC
/opt/texlive/2026/bin
speedy 09:25:08 ~$ kpsewhich -var-value=SELFAUTODIR
/opt/texlive/2026
~~~

The library has a hardwired value for `TEXMFCNF`, which
is evaluated at runtime.

~~~
{$SELFAUTOLOC,$SELFAUTOLOC/share/texmf-local/web2c,$SELFAUTOLOC/share/texmf-dist/web2c,$SELFAUTOLOC/share/texmf/web2c,$SELFAUTOLOC/texmf-local/web2c,$SELFAUTOLOC/texmf-dist/web2c,$SELFAUTOLOC/texmf/web2c,$SELFAUTODIR,$SELFAUTODIR/share/texmf-local/web2c,$SELFAUTODIR/share/texmf-dist/web2c,$SELFAUTODIR/share/texmf/web2c,$SELFAUTODIR/texmf-local/web2c,$SELFAUTODIR/texmf-dist/web2c,$SELFAUTODIR/texmf/web2c,$SELFAUTOGRANDPARENT/texmf-local/web2c,$SELFAUTOPARENT,$SELFAUTOPARENT/share/texmf-local/web2c,$SELFAUTOPARENT/share/texmf-dist/web2c,$SELFAUTOPARENT/share/texmf/web2c,$SELFAUTOPARENT/texmf-local/web2c,$SELFAUTOPARENT/texmf-dist/web2c,$SELFAUTOPARENT/texmf/web2c}
~~~

This can be checked with `kpsewhich`, which is in `/opt/texlive/2026/bin`.

~~~
speedy 09:26:07 ~$ kpsewhich -var-value=TEXMFCNF
{/opt/texlive/2026/bin,/opt/texlive/2026/bin/share/texmf-local/web2c,/opt/texlive/2026/bin/share/texmf-dist/web2c,/opt/texlive/2026/bin/share/texmf/web2c,/opt/texlive/2026/bin/texmf-local/web2c,/opt/texlive/2026/bin/texmf-dist/web2c,/opt/texlive/2026/bin/texmf/web2c,/opt/texlive/2026,/opt/texlive/2026/share/texmf-local/web2c,/opt/texlive/2026/share/texmf-dist/web2c,/opt/texlive/2026/share/texmf/web2c,/opt/texlive/2026/texmf-local/web2c,/opt/texlive/2026/texmf-dist/web2c,/opt/texlive/2026/texmf/web2c,/opt/texmf-local/web2c,/opt/texlive,/opt/texlive/share/texmf-local/web2c,/opt/texlive/share/texmf-dist/web2c,/opt/texlive/share/texmf/web2c,/opt/texlive/texmf-local/web2c,/opt/texlive/texmf-dist/web2c,/opt/texlive/texmf/web2c}
~~~

The final result is

~~~
speedy 09:26:12 ~$ kpsewhich -all texmf.cnf
/opt/texlive/2026/share/texmf-local/web2c/texmf.cnf
/opt/texlive/2026/share/texmf-dist/web2c/texmf.cnf
~~~

The downside of this is that if the executable is not in
`/opt/texlive/2026/bin`, either because it is in `/usr/bin`, such as
`evince`, or in `/home/ralexand/src/dviware`, it won't find
`texmf.cnf`.

To fix this the environment variable `TEXMFCNF` needs to be
set.

~~~
export TEXMFCNF=/opt/texlive/2026/share/texmf-local/web2c:/opt/texlive/2026/share/texmf-dist/web2c
~~~

The bitmap fonts are generated using `Metafont`.  The fonts and
renderer have setable parameters, called a mode.  The default mode is
`ljfour`, which generated 600dpi output.  The output is in `gf` format
but is normally converted to `pk` format by the `mktexpk` utility, and
this is format used by `xdvi` and `dvips`.

It is possible to generate fonts at other resolutions, such as 100dpi
but rendering at a high resolution then downscaling it with an
alpha channel gives cleaner results.

~~~
mktexpk --bdpi 100 --dpi 100 --destdir $PWD cmr10
~~~
