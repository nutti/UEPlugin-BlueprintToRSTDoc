TestActor
==========

Path: /Game/FunctionalTest/TestActor

.. cpp:class:: TestActor : public Actor

   .. cpp:function:: void Construction_Script(exec then)

      Type: Construction script, the place to spawn components and do other setup. @note Name used in CreateBlueprint function

      Category: 

      Access Modifier: Public

      Constant: False

      Flags: Required API, Event, Blueprint Event

      Construction script, the place to spawn components and do other setup. @note Name used in CreateBlueprint function

      :arg then: 
      :type then: exec

   .. cpp:function:: void Function_0(exec then, Vector Input_0, TestStructure Input_1)

      Type: Function 0 Description

      Category: Test

      Access Modifier: Public

      Constant: False

      Flags: Blueprint Callable, Blueprint Event, Blueprint Pure

      Function 0 Description

      :arg then: 
      :type then: exec
      :arg Input_0: 
      :type Input_0: Vector
      :arg Input_1: 
      :type Input_1: TestStructure

   .. cpp:function:: (exec, TestEnumeration, EDOFMode) Function_1(exec then)

      Type: Function 1 Description

      Category: Test

      Access Modifier: Public

      Constant: False

      Flags: Has Out Params, Blueprint Callable, Blueprint Event

      Function 1 Description

      :arg then: 
      :type then: exec
      :returns execute: 
      :rtype execute: exec
      :returns Output_0:  (Default: NewEnumerator0)
      :rtype Output_0: TestEnumeration
      :returns Output_1:  (Default: Default)
      :rtype Output_1: EDOFMode

   .. cpp:function:: (exec, bool, string) Function_2(exec then, int Input_0, int Input_1)

      Type: Function 2 Description

      Category: 

      Access Modifier: Public

      Constant: True

      Flags: Has Out Params, Blueprint Callable, Blueprint Event

      Function 2 Description

      :arg then: 
      :type then: exec
      :arg Input_0: 
      :type Input_0: int
      :arg Input_1: 
      :type Input_1: int
      :returns execute: 
      :rtype execute: exec
      :returns Output_0:  (Default: false)
      :rtype Output_0: bool
      :returns Output_1: 
      :rtype Output_1: string

   .. cpp:function:: void Macro_0()

      Type: Macro 0 Description

      Category: 

      Access Modifier: 

      Constant: False

      Flags: 

      Macro 0 Description

   .. cpp:member:: SceneComponent DefaultSceneRoot

      Category: Default

      Access Modifier: 
      Flags: Blueprint Visible, Zero Constructor, Instanced Reference, Non Transactional, No Destructor, Has Get Value Type Hash
      Lifetime Condition: None

      

   .. cpp:member:: bool Variable_0

      Category: Test

      Access Modifier: 
      Flags: Edit, Blueprint Visible, Zero Constructor, Disable Edit On Instance, Is Plain Old Data, No Destructor, Has Get Value Type Hash
      Lifetime Condition: None

      Variable 0 Description

   .. cpp:member:: Vector Variable_1

      Category: Test

      Access Modifier: 
      Flags: Edit, Blueprint Visible, Net, Zero Constructor, Is Plain Old Data, No Destructor, Has Get Value Type Hash
      Lifetime Condition: Autonomous Only

      Variable 1 Description

   .. cpp:member:: TestStructure Variable_2

      Category: Default

      Access Modifier: 
      Flags: Edit, Blueprint Visible, Disable Edit On Instance, Is Plain Old Data, No Destructor, Has Get Value Type Hash
      Lifetime Condition: None

      Variable 2 Description

   .. cpp:member:: MulticastInlineDelegate Event_0

      Category: Default

      Access Modifier: 
      Flags: Edit, Blueprint Visible, Zero Constructor, Disable Edit On Instance, Blueprint Assignable, Blueprint Callable
      Lifetime Condition: None

      Event 0 Description

