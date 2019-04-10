%%test.erl
-module(test).
-export([get_timestamp/1,
        set_timestamp/2,
        concurrent_set/1]).

get_timestamp(Userid) ->
    case gen_tcp:connect("10.1.1.65", 12004, [binary, {packet, 0}, {active,
                         false}]) of
        {ok, Sock} -> gen_tcp:send(Sock, <<18:32/native, 0:32/native,
                16#2001:16/native, 0:32/native, Userid:32/native>>),
            {ok, Packet} = gen_tcp:recv(Sock, 0),
            <<_:32, _:32, _:16, Status:32/native, _/binary>> = Packet,
            case Status =/= 0 of
                true -> io:format("status_code is ~p~n", [Status]);
                false -> <<_:32, _:32, _:16, _:32, _:32, Timestamp:32/native>> = Packet,
                         io:format("timestamp is ~p~n", [Timestamp])
             end,
            gen_tcp:close(Sock);
        {error, Reason} -> io:format("connect err ~p~n", [Reason])
    end.

set_timestamp(Userid, Timestamp) ->
    case gen_tcp:connect("10.1.1.65", 12004, [binary, {packet, 0}, {active,
                         false}]) of
        {ok, Sock} -> gen_tcp:send(Sock, <<22:32/native, 0:32/native,
                16#2002:16/native, 0:32/native, Userid:32/native, Timestamp:32/native>>),
            {ok, Packet} = gen_tcp:recv(Sock, 0),
            <<_:32, _:32, _:16, Status:32/native, _:32>> = Packet,
            io:format("status_code is ~p~n", [Status]),
            gen_tcp:close(Sock);
        {error, Reason} -> io:format("connect err ~p~n", [Reason])
    end.

concurrent_set(Nloop) when Nloop > 1->
    spawn(fun() -> set_timestamp(Nloop, 1234567) end),
    concurrent_set(Nloop-1);
concurrent_set(1) ->
    spawn(fun() -> set_timestamp(1, 1234567) end).

