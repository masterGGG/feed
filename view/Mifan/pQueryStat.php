<?php
/**
 * Auto generated from statisticPFeed.proto at 2019-06-13 14:07:18
 *
 * mifan package
 */

namespace Mifan {
/**
 * pQueryStat message
 */
class pQueryStat extends \ProtobufMessage
{
    /* Field index constants */
    const LIST = 1;
    const CNT = 2;

    /* @var array Field descriptors */
    protected static $fields = array(
        self::LIST => array(
            'name' => 'list',
            'repeated' => true,
            'type' => '\Mifan\pQueryStat_detail'
        ),
        self::CNT => array(
            'name' => 'cnt',
            'required' => false,
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
        $this->values[self::LIST] = array();
        $this->values[self::CNT] = null;
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
     * Appends value to 'list' list
     *
     * @param \Mifan\pQueryStat_detail $value Value to append
     *
     * @return null
     */
    public function appendList(\Mifan\pQueryStat_detail $value)
    {
        return $this->append(self::LIST, $value);
    }

    /**
     * Clears 'list' list
     *
     * @return null
     */
    public function clearList()
    {
        return $this->clear(self::LIST);
    }

    /**
     * Returns 'list' list
     *
     * @return \Mifan\pQueryStat_detail[]
     */
    public function getList()
    {
        return $this->get(self::LIST);
    }

    /**
     * Returns true if 'list' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasList()
    {
        return count($this->get(self::LIST)) !== 0;
    }

    /**
     * Returns 'list' iterator
     *
     * @return \ArrayIterator
     */
    public function getListIterator()
    {
        return new \ArrayIterator($this->get(self::LIST));
    }

    /**
     * Returns element from 'list' list at given offset
     *
     * @param int $offset Position in list
     *
     * @return \Mifan\pQueryStat_detail
     */
    public function getListAt($offset)
    {
        return $this->get(self::LIST, $offset);
    }

    /**
     * Returns count of 'list' list
     *
     * @return int
     */
    public function getListCount()
    {
        return $this->count(self::LIST);
    }

    /**
     * Sets value of 'cnt' property
     *
     * @param integer $value Property value
     *
     * @return null
     */
    public function setCnt($value)
    {
        return $this->set(self::CNT, $value);
    }

    /**
     * Returns value of 'cnt' property
     *
     * @return integer
     */
    public function getCnt()
    {
        $value = $this->get(self::CNT);
        return $value === null ? (integer)$value : $value;
    }

    /**
     * Returns true if 'cnt' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasCnt()
    {
        return $this->get(self::CNT) !== null;
    }
}
}