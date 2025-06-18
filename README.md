# ndnSIM-KNUST

This repository contains our ndnSIM simulation environment for testing new ideas and modifications.

## Setup Instructions

1. Clone this repository
2. Navigate to ns-3 directory: `cd ns-3`
3. Configure: `sudo ./waf configure --enable-examples`
4. Build: `sudo ./waf`
5. Run example: `NS_LOG=ndn.SpontaneousProducer:ndn.Subscriber ./waf --run ndn-simple`

## Project Structure

- `ns-3/`: Main ns-3 simulator with smartgrid extensions
- `ns-3/src/ndnSIM/`: NDN simulator module
- `ns-3/src/ndnSIM/examples/`: Example scenarios

