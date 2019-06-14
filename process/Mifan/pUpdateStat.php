<?php
/**
 * Auto generated from statisticPFeed.proto at 2019-06-13 15:14:55
 *
 * mifan package
 */

namespace Mifan {
/**
 * pUpdateStat message
 */
class pUpdateStat extends \ProtobufMessage
{
    /* Field index constants */
    const CMD = 1;
    const MIMI = 2;

    /* @var array Field descriptors */
    protected static $fields = array(
        self::CMD => array(
            'name' => 'cmd',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_INT,
        ),
        self::MIMI => array(
            'name' => 'mimi',
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
        $this->values[self::CMD] = null;
        $this->values[self::MIMI] = array();
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
     * Sets value of 'cmd' property
     *
     * @param integer $value Property value
     *
     * @return null
     */
    public function setCmd($value)
    {
        return $this->set(self::CMD, $value);
    }

    /**
     * Returns value of 'cmd' property
     *
     * @return integer
     */
    public function getCmd()
    {
        $value = $this->get(self::CMD);
        return $value === null ? (integer)$value : $value;
    }

    /**
     * Returns true if 'cmd' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasCmd()
    {
        return $this->get(self::CMD) !== null;
    }

    /**
     * Appends value to 'mimi' list
     *
     * @param integer $value Value to append
     *
     * @return null
     */
    public function appendMimi($value)
    {
        return $this->append(self::MIMI, $value);
    }

    /**
     * Clears 'mimi' list
     *
     * @return null
     */
    public function clearMimi()
    {
        return $this->clear(self::MIMI);
    }

    /**
     * Returns 'mimi' list
     *
     * @return integer[]
     */
    public function getMimi()
    {
        return $this->get(self::MIMI);
    }

    /**
     * Returns true if 'mimi' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasMimi()
    {
        return count($this->get(self::MIMI)) !== 0;
    }

    /**
     * Returns 'mimi' iterator
     *
     * @return \ArrayIterator
     */
    public function getMimiIterator()
    {
        return new \ArrayIterator($this->get(self::MIMI));
    }

    /**
     * Returns element from 'mimi' list at given offset
     *
     * @param int $offset Position in list
     *
     * @return integer
     */
    public function getMimiAt($offset)
    {
        return $this->get(self::MIMI, $offset);
    }

    /**
     * Returns count of 'mimi' list
     *
     * @return int
     */
    public function getMimiCount()
    {
        return $this->count(self::MIMI);
    }
}
}