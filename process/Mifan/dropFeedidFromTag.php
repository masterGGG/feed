<?php
/**
 * Auto generated from protoFeedid.proto at 2019-06-10 22:17:50
 *
 * mifan package
 */

namespace Mifan {
/**
 * dropFeedidFromTag message
 */
class dropFeedidFromTag extends \ProtobufMessage
{
    /* Field index constants */
    const FEEDID = 1;
    const TAG = 2;

    /* @var array Field descriptors */
    protected static $fields = array(
        self::FEEDID => array(
            'name' => 'feedid',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_STRING,
        ),
        self::TAG => array(
            'name' => 'tag',
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
        $this->values[self::TAG] = array();
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
     * Appends value to 'tag' list
     *
     * @param integer $value Value to append
     *
     * @return null
     */
    public function appendTag($value)
    {
        return $this->append(self::TAG, $value);
    }

    /**
     * Clears 'tag' list
     *
     * @return null
     */
    public function clearTag()
    {
        return $this->clear(self::TAG);
    }

    /**
     * Returns 'tag' list
     *
     * @return integer[]
     */
    public function getTag()
    {
        return $this->get(self::TAG);
    }

    /**
     * Returns true if 'tag' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasTag()
    {
        return count($this->get(self::TAG)) !== 0;
    }

    /**
     * Returns 'tag' iterator
     *
     * @return \ArrayIterator
     */
    public function getTagIterator()
    {
        return new \ArrayIterator($this->get(self::TAG));
    }

    /**
     * Returns element from 'tag' list at given offset
     *
     * @param int $offset Position in list
     *
     * @return integer
     */
    public function getTagAt($offset)
    {
        return $this->get(self::TAG, $offset);
    }

    /**
     * Returns count of 'tag' list
     *
     * @return int
     */
    public function getTagCount()
    {
        return $this->count(self::TAG);
    }
}
}