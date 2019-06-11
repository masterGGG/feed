<?php
/**
 * Auto generated from protoFeedid.proto at 2019-05-23 16:50:53
 *
 * mifan package
 */

namespace Mifan {
/**
 * feedidToFriend message
 */
class feedidToFriend extends \ProtobufMessage
{
    /* Field index constants */
    const FEEDID = 1;
    const TIME = 2;
    const MIMI = 3;

    /* @var array Field descriptors */
    protected static $fields = array(
        self::FEEDID => array(
            'name' => 'feedid',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_STRING,
        ),
        self::TIME => array(
            'name' => 'time',
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
        $this->values[self::FEEDID] = null;
        $this->values[self::TIME] = null;
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
     * Sets value of 'feedid' property
     *
     * @param string $value Property value
     *
     * @return null
     */
    public function setFeedid($value)
    {
        return $this->set(self::FEEDID, $value);
    }

    /**
     * Returns value of 'feedid' property
     *
     * @return string
     */
    public function getFeedid()
    {
        $value = $this->get(self::FEEDID);
        return $value === null ? (string)$value : $value;
    }

    /**
     * Returns true if 'feedid' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasFeedid()
    {
        return $this->get(self::FEEDID) !== null;
    }

    /**
     * Sets value of 'time' property
     *
     * @param integer $value Property value
     *
     * @return null
     */
    public function setTime($value)
    {
        return $this->set(self::TIME, $value);
    }

    /**
     * Returns value of 'time' property
     *
     * @return integer
     */
    public function getTime()
    {
        $value = $this->get(self::TIME);
        return $value === null ? (integer)$value : $value;
    }

    /**
     * Returns true if 'time' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasTime()
    {
        return $this->get(self::TIME) !== null;
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