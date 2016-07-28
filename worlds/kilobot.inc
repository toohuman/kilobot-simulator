#
# Kilobot model
#

define kb_sensor sensor
(
    # Sense up to 12 cm
    range   [ 0.0 0.12 ]
    pose    [ 0 0 0.0 0]
    size    [ 0.001 0.001 0.001 ]
    # Samples such that a ray guaranteed to intersect kilobot body at maximum range
    samples 32
    # Sense all round. The first and last sense rays are at the extremities of the
    # fov, so to have the 32 samples evenly spaced means that the fov is 360-360/32
    fov     348.75
)

define kb_ranger ranger
(
    kb_sensor( pose [0 0 -0.005 0] )
)


define kb_base position 
(
    color               "grey"		# Default color.
    drive               "omni"		# Differential steering model.
    gui_nose            1           # Draw a nose on the robot so we can see which way it points
    obstacle_return     0           # Can hit things.
    ranger_return       1           # reflects sonar beams
    blob_return         1           # Seen by blobfinders  
    fiducial_return     1           # Seen as "1" fiducial finders
    size                [ 0.030 0.030 0.01 ]
    mass                0.01

    localization "gps"         	  
    localization_origin [0 0 0 0] 	# Start odometry at (0, 0, 0)
    
    # LED
    blinkenlight
    (
        # generic model properties
        size [ 0.01 0.01 0.001 ]
        pose [ 0.005 0.0 0.0 0.0 ]
        # type-specific properties
        dutycycle 1.0
        enabled 1
    )


)


define kilobot kb_base
(
    # This locates the centre of rotation 10 mm behind the centre of
    # the kilobot body
    kb_ranger( pose [0.0 0 0 0] )
    origin          [0.0 0.0 0.0 0.0 ]
    block
    (
        #points 4
        #point[0] [ 0.016 0.016 ]
        #point[1] [ -0.016 0.016 ]
        #point[2] [ -0.016 -0.016 ]
        #point[3] [ 0.016 -0.016 ]

        points  16
        point[0] [ 0.016 0.0 ]
        point[1] [ 0.0147820725202 0.00612293491784 ]
        point[2] [ 0.011313708499 0.011313708499 ]
        point[3] [ 0.00612293491784 0.0147820725202 ]
        point[4] [ 0.0 0.016 ]
        point[5] [-0.00612293491784 0.0147820725202 ]
        point[6] [-0.011313708499 0.011313708499 ]
        point[7] [-0.0147820725202 0.00612293491784 ]
        point[8] [-0.016 0.0 ]
        point[9] [-0.0147820725202 -0.00612293491784 ]
        point[10] [-0.011313708499 -0.011313708499 ]
        point[11] [-0.00612293491784 -0.0147820725202 ]
        point[12] [0.0 -0.016 ]
        point[13] [0.00612293491784 -0.0147820725202 ]
        point[14] [0.011313708499 -0.011313708499 ]
        point[15] [0.0147820725202 -0.00612293491784 ]
        z [ 0.0 0.1 ]
    )
)


# spatial resolution of the underlying occupancy grid. Make 1 mm
resolution 0.001


