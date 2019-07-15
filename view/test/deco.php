<?php
//$decode='8AtTRQAAAAA=';
//$decode='8AtTRQAAAAA=';
$decode='dUbiRAAAAAA=';
echo $decode;
$data=base64_decode($decode);
echo $data;
$magic = unpack("Lm1/Lm2", $data);
var_dump($magic);
?>
