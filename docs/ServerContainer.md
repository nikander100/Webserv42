# ServerContainer

The server contianer is the heart of the project. It manages the server objects, handles the routing and manages an epoll instance that is used to handle the connections.

## Table of Contents

- [Class Name](#class-name)
	- [Table of Contents](#table-of-contents)
	- [Constructor](#constructor)
	- [Methods](#methods)
	- [Properties](#properties)

## Constructor

### `ServerContainer()`

The constructor has no specific input as this is more a factory constructor.

#### Parameters

- `param1`: Description of the parameter.
- `param2`: Description of the parameter.

#### Example


## Methods

### `setupServers()`

Is called by `startservers()
Handles the basic setup for each server 

### `startServers()`