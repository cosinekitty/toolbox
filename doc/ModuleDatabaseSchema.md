# Module Database Schema

## Module

For each module, we want fields that help understand what the ports and parameters mean.

- plugin slug
- plugin brand/name
- width in HP units
- module slug: `widget->getModel()->slug`
- module name: `widget->getModel()->name`
- array of Parameter
- input = array of Port
- output = array of Port

## Parameter

`widget->getParam(paramId)->getParamQuantity()`

- paramId integer
- minValue
- maxValue
- defaultValue
- parameter name
- parameter unit
- (displayBase, displayMultiplier, displayOffset)
- Screen coordinates: box and size

## InputPort, OutputPort

`widget->getInput(portId)->getPortInfo()`

- portId
- name
- description
