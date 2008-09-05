#!/usr/bin/perl
#
# Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
#
# $Id: runexperiment.pl,v 1.8 2006/06/14 15:04:34 tjohnson Exp $
#
# script to run the simulator on a whole pile of computers...
# the computers are sorted into three sets, light medium and heavy.
#  the light jobs (in lightlist) go onto the light computers, 
#  medium jobs on the medium computers, etc.

open(TMP,"countcpus|");
@machinelist=<TMP>;
close(TMP);

foreach $line (@machinelist)
{
	($speed,$addr)=split(/[ \t]/,$line);

	chomp($addr);

	if ($speed < 400)
	{
		@lightmachinelist=(@lightmachinelist,$addr);
		next;
	}
	if ($speed < 995)
	{
		@medmachinelist=(@medmachinelist,$addr);
		next;
	}

	@heavymachinelist=(@heavymachinelist,$addr);
}

#print "light: ",join(" ",@lightmachinelist),"\n";
#print "medium: ",join(" ",@medmachinelist),"\n";
#print "heavy: ",join(" ",@heavymachinelist),"\n";

$duration=10000;
@alglist=("nonailed.interim","nonailed.bft","nailed.interim2","nonailed.interim2");

@lightlist=(
	"manet20.deg4.mob1","manet20.deg4.mob4",
	"manet80.deg4.mob1","manet80.deg4.mob4",
	);

@medlist=(
	"manet80.deg8.mob1","manet80.deg8.mob4",
	"manet200.deg4.mob1","manet200.deg4.mob4",
	);

@heavylist=(
	"manet200.deg8.mob1","manet200.deg8.mob4",
	"manet200.deg12.mob1","manet200.deg12.mob4",
	);


# here endeth the configuration.


sub getheavymachine
{
	$heavypos++;
	if ($heavymachinelist[$heavypos] eq "")
	{
		$heavypos=0;
	}
	$heavymachinelist[$heavypos];
}

sub getmedmachine
{
	$medpos++;
	if ($medmachinelist[$medpos] eq "")
	{
		$medpos=0;
	}
	$medmachinelist[$medpos];
}

sub getlightmachine
{
	$lightpos++;
	if ($lightmachinelist[$lightpos] eq "")
	{
		$lightpos=0;
	}
	$lightmachinelist[$lightpos];
}


$cwd=`pwd`;
chomp($cwd);

foreach $i (@lightlist)
	{ $lightlist{$i}=1; }
foreach $i (@medlist)
	{ $medlist{$i}=1; }
foreach $i (@heavylist)
	{ $heavylist{$i}=1; }

foreach $conf (@lightlist,@medlist, @heavylist)
{
	foreach $alg (@alglist)
	{
		$base="./${conf}.${alg}.res";
		print "mkdir $base\n";

		if ($lightlist{$conf}>0 ) { $host=getlightmachine(); }
		if ($medlist{$conf}>0) { $host=getmedmachine(); }
		if ($heavylist{$conf}>0) { $host=getheavymachine(); }

		$executable=substr($alg,index($alg,".")+1);
		$nail=substr($alg,0,index($alg,"."));
		$conffile="$conf.$nail.conf";

		print "ssh $host \"cd $cwd ; ./$executable ./$conffile $duration > ${base}/foo 2> ${base}/stderr\" < /dev/null &\n";
	}
}
