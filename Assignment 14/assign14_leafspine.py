from mininet.topo import Topo
from mininet.net import Mininet
from mininet.cli import CLI
from mininet.log import setLogLevel, info
from mininet.node import OVSSwitch
from mininet.node import Controller
import sys

class LeafSpineTopo(Topo):
    def build(self, radix=4, hosts_per_leaf=2):

        if radix < 2:
            raise Exception("Radix must be >= 2")

        num_spines = radix // 2
        num_leafs  = radix // 2

        spine_switches = []
        leaf_switches = []

        info("\n*** Creating Spine Switches\n")
        for i in range(num_spines):
            s = self.addSwitch(f'spine{i+1}')
            spine_switches.append(s)

        info("\n*** Creating Leaf Switches\n")
        for i in range(num_leafs):
            l = self.addSwitch(f'leaf{i+1}')
            leaf_switches.append(l)

        info("\n*** Connecting Leaf <--> Spine\n")
        for leaf in leaf_switches:
            for spine in spine_switches:
                self.addLink(leaf, spine)

        info("\n*** Attaching Hosts to Leaf Switches\n")
        host_count = 1
        for leaf in leaf_switches:
            for _ in range(hosts_per_leaf):
                h = self.addHost(f'h{host_count}')
                self.addLink(h, leaf)
                host_count += 1


def run(radix=4, hosts_per_leaf=2):
    topo = LeafSpineTopo(radix=radix, hosts_per_leaf=hosts_per_leaf)
    net = Mininet(topo=topo, switch=OVSSwitch, controller=None, autoStaticArp=True)
    net.start()
    CLI(net)
    net.stop()


if __name__ == "__main__":
    setLogLevel('info')

    # Default values
    radix = 4
    hosts_per_leaf = 2

    if len(sys.argv) >= 2:
        radix = int(sys.argv[1])
    if len(sys.argv) == 3:
        hosts_per_leaf = int(sys.argv[2])

    run(radix, hosts_per_leaf)

