#!/usr/bin/php
<?php

define('KEY_SIZE',8);

function dumpkey($key)
{
    foreach($key as $byte)
		printf('%02X',$byte);
    echo "\n";
}

function permute($key)
{
    $res = array();

    // support 3DES keys of 16 bytes
    if(($i=count($key))>KEY_SIZE)
    {
	foreach(array_chunk($key,KEY_SIZE) as $subkey)
	    $res=array_merge($res,permute($subkey));
	return $res;
    }
    else
	if($i!=KEY_SIZE)
	    exit("key size needs to be multiples of 8 bytes");

    for($i=0;$i<KEY_SIZE;$i++)
    {
		$p=0;
		$mask=0x80>>$i;
		foreach($key as $byte)
		{
			$p>>=1;
			if($byte & $mask)
			$p|=0x80;
		}
		$res[] = $p;
    }
    return $res;
}

function permute_n($key,$n)
{
	while($n--)
		$key = permute($key);
	return $key;
}

function permute_reverse($key)
{
	return permute_n($key,3);
}

function crc($key)
{
    $keysize = count($key);
    $res = array();
    $crc=0;
    for($i=0;$i<$keysize;$i++)
    {
	if(($i & 7)==7)
	{
	    $res[]=$crc^0xFF;
	    $crc=0;
	}
	else
	{
	    $res[]=$key[$i];
	    $crc^=$key[$i];
	}
    }

    return $res;
}


function generate($key)
{
    echo "    input key: ";
    dumpkey($key);

    echo " permuted key: ";
    $permuted=permute($key);
    dumpkey($permuted);

    echo "   CRC'ed key: ";
    $crc=crc($permuted);
    dumpkey($crc);

    return $crc;
}

function shave($key)
{
    $res = array();

    foreach($key as $keyvalue)
		$res[]=$keyvalue&0xFE;

    return $res;
}

function generate_rev($key)
{
    echo "    input permuted key: ";
    dumpkey($key);

    echo "        unpermuted key: ";
    $key=permute_reverse($key);
    dumpkey($key);

    echo "            shaved key: ";
    $key=shave($key);
    dumpkey($key);

    return $key;
}

function str2hex($keystr)
{
    $key=array();
    foreach(str_split($keystr,2) as $hex)
	$key[]=hexdec($hex);
    return $key;
}

function show_usage()
{
    global $argv;
    echo "$argv[0] [-r|-f] 012345679ABCDEF\n";
}

if($argc==3)
{
    $key=str2hex($argv[2]);

    switch($argv[1])
    {
	case '-f':
	    generate($key);
	    break;
	case '-r':
	    generate_rev($key);
	    break;
	default:
	    show_usage();
    }
}
else
    show_usage();
?>
