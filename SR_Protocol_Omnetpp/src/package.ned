package sr_protocol_omnetpp;

@license(LGPL);
//
// TODO documentation
//
network Network
{
     parameters:
        int WS ;
        int SN;
        double TO ;
        double PT ;
        double TD ;
        double ED ;
        double DD ;
        double LP ;
    @display("bgb=413,227");
    submodules:
        node0: Node {
            @display("p=45,70");
        }
        node1: Node {
            @display("p=356,70");
        }
        coordinator: Coordinator {
            @display("p=201,174");
        }
    connections:
        node0.dataGate <--> node1.dataGate;
        coordinator.controlGate[1] --> node1.controlGate;
        coordinator.controlGate[0] --> node0.controlGate;
}

