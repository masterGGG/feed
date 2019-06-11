<?php
/**
 * Auto generated from protoFeedid.proto at 2019-06-10 22:17:50
 *
 * mifan package
 */

namespace Mifan {
/**
 * feedidFromMine message
 */
class feedidFromMine extends \ProtobufMessage
{
    /* Field index constants */
    const BEGINTIME = 1;
    const ENDTIME = 2;
    const CNT = 3;

    /* @var array Field descriptors */
    protected static $fields = array(
        self::BEGINTIME => array(
            'name' => 'beginTime',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_INT,
        ),
        self::ENDTIME => array(
            'name' => 'endTime',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_INT,
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
        $this->values[self::BEGINTIME] = null;
        $this->values[self::ENDTIME] = null;
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
     * Sets value of 'beginTime' property
     *
     * @param integer $value Property value
     *
     * @return null
     */
    public function setBeginTime($value)
    {
        return $this->set(self::BEGINTIME, $value);
    }

    /**
     * Returns value of 'beginTime' property
     *
     * @return integer
     */
    public function getBeginTime()
    {
        $value = $this->get(self::BEGINTIME);
        return $value === null ? (integer)$value : $value;
    }

    /**
     * Returns true if 'beginTime' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasBeginTime()
    {
        return $this->get(self::BEGINTIME) !== null;
    }

    /**
     * Sets value of 'endTime' property
     *
     * @param integer $value Property value
     *
     * @return null
     */
    public function setEndTime($value)
    {
        return $this->set(self::ENDTIME, $value);
    }

    /**
     * Returns value of 'endTime' property
     *
     * @return integer
     */
    public function getEndTime()
    {
        $value = $this->get(self::ENDTIME);
        return $value === null ? (integer)$value : $value;
    }

    /**
     * Returns true if 'endTime' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasEndTime()
    {
        return $this->get(self::ENDTIME) !== null;
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