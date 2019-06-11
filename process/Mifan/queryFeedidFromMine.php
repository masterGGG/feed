<?php
/**
 * Auto generated from protoFeedid.proto at 2019-06-10 22:17:50
 *
 * mifan package
 */

namespace Mifan {
/**
 * queryFeedidFromMine message
 */
class queryFeedidFromMine extends \ProtobufMessage
{
    /* Field index constants */
    const FEEDID = 1;

    /* @var array Field descriptors */
    protected static $fields = array(
        self::FEEDID => array(
            'name' => 'feedid',
            'repeated' => true,
            'type' => \ProtobufMessage::PB_TYPE_STRING,
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
        $this->values[self::FEEDID] = array();
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
     * Appends value to 'feedid' list
     *
     * @param string $value Value to append
     *
     * @return null
     */
    public function appendFeedid($value)
    {
        return $this->append(self::FEEDID, $value);
    }

    /**
     * Clears 'feedid' list
     *
     * @return null
     */
    public function clearFeedid()
    {
        return $this->clear(self::FEEDID);
    }

    /**
     * Returns 'feedid' list
     *
     * @return string[]
     */
    public function getFeedid()
    {
        return $this->get(self::FEEDID);
    }

    /**
     * Returns true if 'feedid' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasFeedid()
    {
        return count($this->get(self::FEEDID)) !== 0;
    }

    /**
     * Returns 'feedid' iterator
     *
     * @return \ArrayIterator
     */
    public function getFeedidIterator()
    {
        return new \ArrayIterator($this->get(self::FEEDID));
    }

    /**
     * Returns element from 'feedid' list at given offset
     *
     * @param int $offset Position in list
     *
     * @return string
     */
    public function getFeedidAt($offset)
    {
        return $this->get(self::FEEDID, $offset);
    }

    /**
     * Returns count of 'feedid' list
     *
     * @return int
     */
    public function getFeedidCount()
    {
        return $this->count(self::FEEDID);
    }
}
}