<?php
require 'friendFeedList.php';

$tmp = new \Mifan\friendFeedList();

$tmp->setFeedid('12345678:123444:0x23:123124');
$tmp->appendMimi(1);
$tmp->appendMimi(2);
$tmp->appendMimi(3);
$tmp->appendMimi(4);
$tmp->appendMimi(5);

$tmp->dump();

//echo $tmp->serializeToString() > ./xxx
$file=fopen("xxx","w") or exit("无法打开文件!");
echo strlen($tmp->serializeToString());
fwrite($file, $tmp->serializeToString());
fclose($file);
$rfile=fopen("xxx","r") or exit("无法打开文件!");

$buf = fread($rfile, 1024);
echo strlen($buf);
fclose($rfile);

$n = new \Mifan\friendFeedList();
$n->ParseFromString($buf);
$n->dump();
?>
