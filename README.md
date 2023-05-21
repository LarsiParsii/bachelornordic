# bachelornordic
Repository for group E2316's bachelor project (IELET2900)
https://github.com/LarsiParsii/bachelornordic

By:
Lars,
Njål,
Marius, &
Jonathan


FILES
- main_unit			Contain the code for the main unit. Comprises CoAP, GNSS, LTE, and sensor functionality. Build for nrf9160dk_nrf9160_ns.
- vessel_unit		Contain code for the vessel unit. It has CoAP, GNSS, and LTE functionality, but is not able to read the Main Unit's CoAP resource. Build for nrf9160dk_nrf9160_ns.
- main_unit_ble		Contains code meant for the main unit. It acts as a working Bluetooth LE peripheral, broadcasting its GSS service along with Device Information Service (DIS). Test with the nRF Connect app. Build for nrf52840dk_nrf52840.
- vessel_unit_ble	Contain code meant for the vessel unit. It acts as a Bleutooth LE central, automatically connecting to main units over BLE which have a specific model number in their Device Information Service (DIS). Build for nrf52840dk_nrf52840.


NOTES
To browse the projects in VS Code:
1) Open VS Code.
2) Choose File > Open folder > Select either the parent folder where this file resides, or the child folders containing each project.
3) Source files are found in the folder "src".

ACKNOWLEDGEMENT
Some of the code in this repository is sourced from Nordic Semiconductor DevAcademy (https://academy.nordicsemi.com/), and some is from the Zephyr & nRF SDK sample library.
We've been encouraged to use these codebases as a starting point for out own solutions, and to familiarise ourselves with the nRF and Zephyr development enviroment.
As such, we do not claim ownership over any of the code we used that have been previosuly published to said platform and resources.

All respective rights and copyright pertaining to these sourced codes belong to their original authors, Nordic Semiconductor and the Zephyr & nRF SDK sample library.
Our contribution to this repository primarily lies in our modifications, adaptations, and unique code that we've written to realize our specific project objectives.