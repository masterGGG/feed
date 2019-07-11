<?php
/**
 * Auto generated from statisticPFeed.proto at 2019-07-11 11:56:49
 *
 * mifan package
 */

namespace Mifan {
/**
 * pQueryCntRes message
 */
class pQueryCntRes extends \ProtobufMessage
{
    /* Field index constants */
    const CNT = 1;

    /* @var array Field descriptors */
    protected static $fields = array(
        self::CNT => array(
            'name' => 'cnt',
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
        $this->values[self::CNT] = array();
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
     * Appends value to 'cnt' list
     *
     * @param integer $value Value to append
     *
     * @return null
     */
    public function appendCnt($value)
    {
        return $this->append(self::CNT, $value);
    }

    /**
     * Clears 'cnt' list
     *
     * @return null
     */
    public function clearCnt()
    {
        return $this->clear(self::CNT);
    }

    /**
     * Returns 'cnt' list
     *
     * @return integer[]
     */
    public function getCnt()
    {
        return $this->get(self::CNT);
    }

    /**
     * Returns true if 'cnt' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasCnt()
    {
        return count($this->get(self::CNT)) !== 0;
    }

    /**
     * Returns 'cnt' iterator
     *
     * @return \ArrayIterator
     */
    public function getCntIterator()
    {
        return new \ArrayIterator($this->get(self::CNT));
    }

    /**
     * Returns element from 'cnt' list at given offset
     *
     * @param int $offset Position in list
     *
     * @return integer
     */
    public function getCntAt($offset)
    {
        return $this->get(self::CNT, $offset);
    }

    /**
     * Returns count of 'cnt' list
     *
     * @return int
     */
    public function getCntCount()
    {
        return $this->count(self::CNT);
    }
}
}