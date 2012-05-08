#
# (c) Copyright 2012, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# Horibly hacky code to sort stuff using the not yet ready for prime-time data series server

use lib "/home/anderse/build/opt-debian-6.0-x86_64/DataSeries/src/server/gen-perl";
use lib "$ENV{HOME}/projects/thrift/lib/perl/lib";

use Thrift::BinaryProtocol;
use Thrift::Socket;
use Thrift::BufferedTransport;

use DataSeriesServer;

my $socket = new Thrift::Socket('localhost', 49476);
$socket->setRecvTimeout(1000*1000);
my $transport = new Thrift::BufferedTransport($socket, 4096, 4096);
my $protocol = new Thrift::BinaryProtocol($transport);
my $client = new DataSeriesServerClient($protocol);

$transport->open();
$client->ping();

if (!$client->hasTable('edges')) {
    $client->importDataSeriesFiles(["/tmp/x.ds"],
                                   'edge_list_graph', 'edges');
}

my $sc_0 = new SortColumn({ 'column' => 'a', 'sort_mode' => SortMode::SM_Ascending });
my $sc_1 = new SortColumn({ 'column' => 'b', 'sort_mode' => SortMode::SM_Ascending });

$client->sortTable('edges', 'edges-sorted', [ $sc_0, $sc_1 ]);
