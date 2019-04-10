<?php
/* vim: set expandtab tabstop=4 softtabstop=4 shiftwidth=4: */
/**
 * @file netclient_class.php
 * @author richard <richard@taomee.com>
 * @date 2011-06-10
 */

require_once('function.php');

class netclient 
{
    var $host = '';
    var $port = 0;
    var $sock = NULL;

    function __construct($host, $port) 
    {
        $this->host = $host;
        $this->port = $port;
    }

    function open_conn($time_out_sec) 
    {
        if (($this->sock = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP)) === FALSE) {
            do_log('error', "open_conn: socket_create: " . socket_strerror(socket_last_error()));
            return FALSE;
        }

        if (@socket_set_nonblock($this->sock) === FALSE) {
            do_log('error', "open_conn: socket_set_nonblock: " . socket_strerror(socket_last_error($this->sock)));
            return FALSE;
        }
    
        if (@socket_connect($this->sock, $this->host, $this->port) === FALSE) {
            if (socket_last_error($this->sock) != SOCKET_EINPROGRESS) {
                do_log('error', "open_conn: socket_set_nonblock: " . socket_strerror(socket_last_error($this->sock)));
                return FALSE;
            }
        }
     
        $read_fd_set = NULL;
        $write_fd_set = array($this->sock);
        $except_fd_set = array($this->sock);

        $rv_0 = @socket_select($read_fd_set, $write_fd_set, $except_fd_set, $time_out_sec);
        if ($rv_0 === FALSE) {
            do_log('error', "open_conn: socket_select: " . socket_strerror(socket_last_error()));
            return FALSE;
        } else if ($rv_0 === 0) {
            do_log('error', "open_conn: socket_select: timeout");
            return FALSE;
        }

        // $rv_0 > 0
        if (count($except_fd_set) > 0 && $except_fd_set[0] === $this->sock) {
            do_log('error', "open_conn: socket_select: " . socket_strerror(socket_last_error($this->sock)));
            return FALSE;
        }

        if (count($write_fd_set) > 0 && $write_fd_set[0] === $this->sock) {
            $rv_1 = @socket_get_option($this->sock, SOL_SOCKET, SO_ERROR);
            if ($rv_1 !== 0) {
                do_log('error', "open_conn: socket_select: " . socket_strerror($rv_1));
                return FALSE;
            }

            return TRUE;
        }

        do_log('error', 'open_conn: connect: it should never come here');

        return FALSE;
    }

    function send_rqst($rqst_msg, $time_out_sec)
    {
        $rqst_msg_rmng_len = strlen($rqst_msg);
        $resp_msg = '';

        for (;;) {
            $read_fd_set = array($this->sock);
            $write_fd_set = NULL;
            $except_fd_set = array($this->sock);
            if ($rqst_msg_rmng_len > 0) {
                $write_fd_set = array($this->sock);
            }
            
            $rv_0 = @socket_select($read_fd_set, $write_fd_set, $except_fd_set, $time_out_sec);   // TODO
            if ($rv_0 === FALSE) {
                do_log('error', "ERROR: socket_select: " . socket_strerror(socket_last_error()));
                return FALSE;
            } else if ($rv_0 === 0) {
                do_log('error', "ERROR: socket_select: timeout");
                return FALSE;
            }

            // $rv_0 > 0
            if (count($read_fd_set) > 0 && $read_fd_set[0] === $this->sock) {
                    $rv_1 = @socket_read($this->sock, 4096);
                    if ($rv_1 === FALSE) {
                        do_log('error', "ERROR: socket_read: " . socket_strerror(socket_last_error($this->sock)));
                        return FALSE;
                    }

                    $resp_msg .= $rv_1;

                    if (strlen($resp_msg) >= 4) {
                        $rv_unpack = unpack('Llen', $resp_msg);
                        if ($rv_unpack['len'] > strlen($resp_msg)) {
                            // do nothing
                        } else if ($rv_unpack['len'] == strlen($resp_msg)) {
                            return $resp_msg;
                        } else {
                            do_log('error', $rv_unpack);
                            do_log('error', strlen($resp_msg));
                            return FALSE;
                        }
                    }
            }
            if (count($write_fd_set) > 0 && $write_fd_set[0] === $this->sock) {
                if ($rqst_msg_rmng_len > 0) {
                    $rv_2 = @socket_write($this->sock, $rqst_msg);
                    if ($rv_2 === FALSE) {
                        return FALSE;
                    }
                    $rqst_msg = substr($rqst_msg, $rv_2);
                    $rqst_msg_rmng_len -= $rv_2;
                }
            }
            if (count($except_fd_set) > 0 && $except_fd_set[0] === $this->sock) {
                do_log('error', "ERROR: socket_select: " . socket_strerror(socket_last_error($this->sock)));
                return FALSE;
            }
        }

        return 0;
    }
    
    function close_conn()
    {
        @socket_close($this->sock);
        $this->sock = NULL;
    }
}
