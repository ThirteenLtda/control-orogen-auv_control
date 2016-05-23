require 'minitest/spec'
require 'orocos/test/component'
require 'minitest/autorun'


describe 'auv_control::ThrusterForce2BodyEffort configuration' do
  include Orocos::Test::Component
  start 'thruster_force_2_body_effort', 'auv_control::ThrusterForce2BodyEffort' => 'thruster_force_2_body_effort'
  reader 'thruster_force_2_body_effort', 'body_efforts', :attr_name => 'body_efforts'
  writer 'thruster_force_2_body_effort', 'thruster_forces', :attr_name => 'thruster_forces'


  it 'testing thruster surge and yaw force values' do

    thruster_force_2_body_effort.apply_conf_file("auv_control::ThrusterForce2BodyEffort.yml")

    thruster_force_2_body_effort.configure  
    thruster_force_2_body_effort.start  

    sample = thruster_force_2_body_effort.thruster_forces.new_sample
      
    thruster1 = Types::Base::JointState.new
    thruster2 = Types::Base::JointState.new
    thruster3 = Types::Base::JointState.new
    thruster4 = Types::Base::JointState.new
    thruster5 = Types::Base::JointState.new
    thruster6 = Types::Base::JointState.new
    thruster1.effort = 30
    thruster2.effort = 30
    thruster3.effort = 0
    thruster4.effort = 0
    thruster5.effort = 50
    thruster6.effort = 50
    
    sample.elements = [thruster1, thruster2, thruster3, thruster4, thruster5, thruster6]

    thruster_forces.write sample 

    data = assert_has_one_new_sample body_efforts, 1

    assert (data.linear[0] - 60).abs < 0.001, "wrong expected surge value"
    assert (data.linear[1] - 0.0).abs < 0.001, "wrong expected sway value"
    assert (data.linear[2] + 100).abs < 0.001, "wrong expected heave value"
    assert (data.angular[2] - 0.0).abs < 0.001, "wrong expected yaw value"
  end

  it 'testing thruster sway force values' do

    thruster_force_2_body_effort.apply_conf_file("auv_control::ThrusterForce2BodyEffort.yml")

    thruster_force_2_body_effort.configure  
    thruster_force_2_body_effort.start  

    sample = thruster_force_2_body_effort.thruster_forces.new_sample
      
    thruster1 = Types::Base::JointState.new
    thruster2 = Types::Base::JointState.new
    thruster3 = Types::Base::JointState.new
    thruster4 = Types::Base::JointState.new
    thruster5 = Types::Base::JointState.new
    thruster6 = Types::Base::JointState.new
    thruster1.effort = 0
    thruster2.effort = 0
    thruster3.effort = 25
    thruster4.effort = 32
    thruster5.effort = 0
    thruster6.effort = 0
    
    sample.elements = [thruster1, thruster2, thruster3, thruster4, thruster5, thruster6]

    thruster_forces.write sample 

    data = assert_has_one_new_sample body_efforts, 1

    assert (data.linear[0] - 0.0).abs < 0.001, "wrong expected surge value"
    assert (data.linear[1] + 57).abs < 0.001, "wrong expected sway value"
    assert (data.linear[2] + 0.0).abs < 0.001, "wrong expected heave value"
    assert (data.angular[2] - 15.615).abs < 0.001, "wrong expected yaw value"
  end

end

