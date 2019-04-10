<?php
class feedid
{
	public $mimi;
	public $cmd_id;
	public $app_id;
	public $version;
	public $timestamp;

	public function to_binary()
	{
		$ret = pack('SLCL', $this->cmd_id, $this->mimi, $this->version, $this->timestamp);
		return $ret;
	}
}
