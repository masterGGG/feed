<?php
/**
 * Auto generated from statisticPFeed.proto at 2019-07-11 11:56:49
 *
 * mifan package
 */

namespace Mifan {
/**
 * pQueryCnt message
 */
class pQueryCnt extends \ProtobufMessage
{
    /* Field index constants */
    const MIMI = 1;
    const CMD = 2;

    /* @var array Field descriptors */
    protected static $fields = array(
        self::MIMI => array(
            'name' => 'mimi',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_INT,
        ),
        self::CMD => array(
            'name' => 'cmd',
            'repeated' => true,
            'type' => \ProtobufMessage::PB_TYPE_INT,
        ),
    );

    /**
     * Constructs new message container and clears its internal state
     */
    public function __construct()
    {
        $this->reset();
    }

    /**
     * Clears message values and sets default ones
     *
     * @return null
     */
    public function reset()
    {
        $this->values[self::MIMI] = null;
        $this->values[self::CMD] = array();
    }

    /**
     * Returns field descriptors
     *
     * @return array
     */
    public function fields()
    {
        return self::$fields;
    }

    /**
     * Sets value of 'mimi' property
     *
     * @param integer $value Property value
     *
     * @return null
     */
    public function setMimi($value)
    {
        return $this->set(self::MIMI, $value);
    }

    /**
     * Returns value of 'mimi' property
     *
     * @return integer
     */
    public function getMimi()
    {
        $value = $this->get(self::MIMI);
        return $value === null ? (integer)$value : $value;
    }

    /**
     * Returns true if 'mimi' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasMimi()
    {
        return $this->get(self::MIMI) !== null;
    }

    /**
     * Appends value to 'cmd' list
     *
     * @param integer $value Value to append
     *
     * @return null
     */
    public function appendCmd($value)
    {
        return $this->append(self::CMD, $value);
    }

    /**
     * Clears 'cmd' list
     *
     * @return null
     */
    public function clearCmd()
    {
        return $this->clear(self::CMD);
    }

    /**
     * Returns 'cmd' list
     *
     * @return integer[]
     */
    public function getCmd()
    {
        return $this->get(self::CMD);
    }

    /**
     * Returns true if 'cmd' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasCmd()
    {
        return count($this->get(self::CMD)) !== 0;
    }

    /**
     * Returns 'cmd' iterator
     *
     * @return \ArrayIterator
     */
    public function getCmdIterator()
    {
        return new \ArrayIterator($this->get(self::CMD));
    }

    /**
     * Returns element from 'cmd' list at given offset
     *
     * @param int $offset Position in list
     *
     * @return integer
     */
    public function getCmdAt($offset)
    {
        return $this->get(self::CMD, $offset);
    }

    /**
     * Returns count of 'cmd' list
     *
     * @return int
     */
    public function getCmdCount()
    {
        return $this->count(self::CMD);
    }
}
}