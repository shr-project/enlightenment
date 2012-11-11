#!/usr/bin/perl

use strict;

use Getopt::Long;

my $ME = $0; $ME =~ s{.*/}{};

my $BORDER   =     0;
my $KEY      = undef;
my $ORIENT   =   "v";

my $BASE     =     0;
my $VERBOSE  =     0;
my $UNDER    =     1;

my (%ID, $X0, $Y0, $H, $W, @DNUMs);
my $last_dnum = -1;

GetOptions(
    "border=i" => \$BORDER,
    "key=i"    => \$KEY,
    "orient=s" => \$ORIENT,
    verbose    => \$VERBOSE,
    help       => sub { usage() },
) or exit 2;

sub usage {
    print <<Usage;
Usage: $ME [options]

Make all e16 desktop pagers the same size and arrange them in
a line or grid.  The "key" pager keeps its size and location.
All the other pagers are adjusted to match it.  The default
is to key off the most recently adjusted pager.  The default
orientiation is a vertical line.

Options:
  -b, --border=N   Put N pixels between pagers
  -h, --help       Show this usage
  -k, --key=N      Key off of pager N (-1 for max)
  -v, --verbose    Print more.

  -o, --orient={v|h}[N]

Orientation:

  v    Vertical line
  h    horizontal line
  h2   grid with 2 rows
  h3   grid with 3 rows
  v2   grid with 2 columns
  ... and so on

Usage
    exit;
}

$ORIENT =~ m/^[vVhH]\d*$/ or do {
    warn qq{Invalid orientation "$ORIENT".\n};
    die  qq{Expected a "v" or an "h" followed by an optional number.\n};
};

$ORIENT =~ s/(\d+)// and $BASE = $1;

for my $wind (`eesh wl full`) {
    my ($id, $name, undef, undef, undef, $geo) = split(/\s*:\s*/, $wind);
    chomp $id;
    $name =~ /^Pager-(\d)$/ or next;
    #print "$id, $name, $geo\;
    my $dnum = $1;

    defined $KEY or $KEY = $dnum;

    $id =~ s/\s+$//;
    $ID{$dnum} = $id;
    push @DNUMs, $dnum;
    if ( $KEY < 0 ) {
        $dnum > $last_dnum or next;
    }
    else {
        $dnum == $KEY or next;
    }

    $last_dnum = $dnum;

    ($X0, $Y0, my $size, undef) = split( /\s+/, $geo);
    ($W, $H) = split( /x/, $size);
    #print "$dnum, $id, $X0, $Y0, $W, $H\n";
}

@DNUMs      or die "No pagers found in window list! (eesh wl full)\n";
defined $X0 or die "Key pager was not found!\n";

$KEY < 0 and $KEY = $last_dnum;

my $y = $Y0;
my $x = $X0;
my $mod_x = 0;
my $mod_y = 0;

if ($BASE > 0) {
    $ORIENT eq "v" and ($mod_x, $mod_y) = modular( $KEY, $BASE);
    $ORIENT eq "h" and ($mod_y, $mod_x) = modular( $KEY, $BASE);
    $X0 -= $mod_x * ($W + $BORDER);
    $Y0 -= $mod_y * ($H + $BORDER);
}

$VERBOSE and printf "%8s %8s %8s\n", qw/pager x-pos y-pos/;

for my $dnum (sort {$a <=> $b} @DNUMs) {
    my $id = $ID{$dnum};

    my $offset = $dnum - $KEY;

    if ($BASE > 0) {
        $ORIENT eq "v" and ($mod_x, $mod_y) = modular( $dnum, $BASE);
        $ORIENT eq "h" and ($mod_y, $mod_x) = modular( $dnum, $BASE);
    }
    else {
        $ORIENT eq "v" and $mod_y = $offset;
        $ORIENT eq "h" and $mod_x = $offset;
    }

    $VERBOSE and printf "%8d %8d %8d\n", $dnum, $mod_x, $mod_y;

    #$dnum == $KEY and next;

    $x = $X0 + $mod_x * ($W + $BORDER);
    $y = $Y0 + $mod_y * ($H + $BORDER);
    system("eesh wop $id size $W");
    system("eesh wop $id move $x $y");
    $UNDER and system("eesh wop $id layer 1");

}

sub modular {
    my ($offset, $base) = @_;
    my $x = $offset % $base;
    my $y = sprintf "%d", ($offset - $x) / $base;
    return ($x, $y);
}
