#! /usr/bin/php5
<?

$report_forms = array('outbox_server'=>0,
                      'filter'=>0,
                      'sort'=>0,
                      'storage_server'=>0,
                      'limit_combine'=>0,
                      'total'=>0,
                      'number'=>0);
//array(50,100,200,500,1000,2000)
//0~50,51~100......1001~2000,2001~max
$report_forms_distribute = array(5,10,20,35,50,100,200,500,1000,2000);

function create_forms_distribute($distribute, $key, $json_data, &$report_forms)
{
    $count_distribute = count($distribute);
    if ($key == 'storage_server_time')
    {
        $time = $json_data[$key]['time'];
    }
    else if ($key == 'total')
    {
        $time = $json_data[$key]['time'];
    }
    else
    {
        $time = $json_data[$key];
    }
   
    $i = 0; 
    while ($count_distribute)
    {
        if ($time <= $distribute[$i])
        {
            break; 
        } 
        $i++;
        $count_distribute--;
    }

    $report_forms[$i]['time'] += $time;
    $report_forms[$i]['count']++;
}

$file_num = $argc - 1;
$i = 1;

while($file_num)
{
$fd = fopen($argv[$i], "r");
if ($fd)
{
    while (($buffer = fgets($fd)) !== FALSE)
    {
        $line_data = json_decode(trim($buffer), true);
        $report_forms['outbox_server'] += $line_data['outbox_server_time'];
        $report_forms['filter'] += $line_data['filter_time'];
        $report_forms['sort'] += $line_data['sort_time'];
        $report_forms['storage_server'] += $line_data['storage_server_time']['time'];
        $report_forms['limit_combine'] += $line_data['limit_combine_time'];
        $report_forms['total'] += $line_data['total']['time'];
        $report_forms['number']++;
        create_forms_distribute($report_forms_distribute, 'total', $line_data, $total_dis);
        create_forms_distribute($report_forms_distribute, 'outbox_server_time', $line_data, $outbox_dis);
        create_forms_distribute($report_forms_distribute, 'filter_time', $line_data, $filter_dis);
        create_forms_distribute($report_forms_distribute, 'sort_time', $line_data, $sort_dis);
        create_forms_distribute($report_forms_distribute, 'storage_server_time', $line_data, $storage_dis);
        create_forms_distribute($report_forms_distribute, 'limit_combine_time', $line_data, $limit_combine_dis);
    }
    if (!feof($fd))
    {
        echo "read data from file {$argv[$i]} error\n";
        exit(-1);
    }
}

$i++;
$file_num--;
}

//格式化输出
$outbox_server_avg = $report_forms['outbox_server'] / $report_forms['number'];
$filter_avg = $report_forms['filter'] / $report_forms['number'];
$sort_avg = $report_forms['sort'] / $report_forms['number'];
$storage_server_avg = $report_forms['storage_server'] / $report_forms['number'];
$limit_combine_avg = $report_forms['limit_combine'] / $report_forms['number'];
$other_avg = ($report_forms['total'] - ($report_forms['outbox_server'] + $report_forms['filter'] + $report_forms['sort'] + $report_forms['storage_server'] + $report_forms['limit_combine'])) / $report_forms['number'];
$total_avg = $report_forms['total'] / $report_forms['number'];
echo "-------------------------------------------------------------------\n";
echo "项目 total    | avg(time) 单位毫秒   |  百分比(%)\n";
echo "-------------------------------------------------------------------\n";
echo "outbox_server | " . $outbox_server_avg ."   |  ".$outbox_server_avg / $total_avg * 100 . "%\n";
echo "filter        | " . $filter_avg."   |  ".$filter_avg / $total_avg * 100 . "%\n";
echo "sort          | " . $sort_avg."   |  ".$sort_avg / $total_avg * 100 . "%\n";
echo "storage_server| " . $storage_server_avg."   |  ".$storage_server_avg / $total_avg * 100 . "%\n";
echo "limit_combine | " . $limit_combine_avg."   |  ".$limit_combine_avg / $total_avg * 100 . "%\n";
echo "other         | " . $other_avg."   |  ". $other_avg / $total_avg * 100 . "%\n";
echo "total         | " . $total_avg."   |  100%\n";
echo "-------------------------------------------------------------------\n";
echo "request_number|             {$report_forms['number']}                \n";
echo "-------------------------------------------------------------------\n";

echo "\n";
echo "\n";

function distribute_print($key)
{
    global $report_forms_distribute;
    echo "\n";
    echo "-------------------------------------------------------------------\n";
    echo "项目              | avg(time) 单位毫秒 |  请求次数  |   百分比(%)\n";
    echo "-------------------------------------------------------------------\n";
    $total_number = 0;
    for ($i = 0; $i < count($report_forms_distribute) + 1; $i++)
    {
        $total_number += $key[$i]['count'];  
    } 
    for ($i = 0; $i < count($report_forms_distribute) + 1; $i++)
    {
        if ($key[$i]['count'] == 0)
        {
            echo $report_forms_distribute[$i] . " | " ."0" . " | " . 0 . " | " . $key[$i]['count'] / $total_number * 100 . "%\n";
        }
        else
        {
        echo $report_forms_distribute[$i] . " | " .$key[$i]['time'] / $key[$i]['count'] . " | " . $key[$i]['count'] . " | " . $key[$i]['count'] / $total_number * 100 . "%\n";
        }
    } 
    echo "-------------------------------------------------------------------\n";
    echo "\n";
}

echo "total\n";
distribute_print($total_dis);
echo "outbox\n";
distribute_print($outbox_dis);
echo "filter\n";
distribute_print($filter_dis);
echo "sort\n";
distribute_print($sort_dis);
echo "storage\n";
distribute_print($storage_dis);
echo "limit_combine\n";
distribute_print($limit_combine_dis);

