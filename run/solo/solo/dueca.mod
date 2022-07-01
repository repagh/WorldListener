;; -*-scheme-*-
;; this is an example dueca.mod file, for you to start out with and adapt 
;; according to your needs. Note that you need a dueca.mod file only for the 
;; node with number 0

;; NOTE: for new guile (from openSUSE 12.1 onwards), you may only once use
;; (define ...). For modifying an already defined value, used (set! ...)

;; in general, it is a good idea to clearly document your set up
;; this is an excellent place. 
;; node set-up
(define ecs-node 0)    ; base node

;; priority set-up
; normal nodes: 0 administration
;               1 simulation, unpackers
;               2 communication
;               3 ticker

; administration priority. Run the interface and logging here
(define admin-priority (make-priority-spec 0 0))

; priority of simulation, just above adiminstration
(define sim-priority (make-priority-spec 1 0))

; nodes with a different priority scheme
; control loading node has 0, 1 and 2 as above and furthermore
;               3 stick priority
;               4 ticker priority
; priority of the stick. Higher than prio of communication
(define stick-priority (make-priority-spec 3 0)) 

; timing set-up
; timing of the stick calculations. Assuming 100 usec ticks, this gives 2000 Hz
(define stick-timing (make-time-spec 0 5))

; this is normally 100, giving 100 Hz timing
(define sim-timing (make-time-spec 0 100))

;; for now, display on 50 Hz
(define display-timing (make-time-spec 0 200))

;; log a bit more economical, 25 Hz
(define log-timing (make-time-spec 0 400))

;;; the modules needed for dueca itself
(dueca-list
  (make-entity "dueca"
	       (if (equal? 0 this-node-id)
		   (list
		    (make-module 'dusime "" admin-priority)
		    (make-module 'dueca-view "" admin-priority)
		    (make-module 'activity-view "" admin-priority)
		    (make-module 'timing-view "" admin-priority)
		    (make-module 'log-view "" admin-priority)
		    ) 
		 (list) 
		 )
	       )
  )

;;; the modules for your application(s)
(define audio
  (make-entity
   "ph-sound"
   (if (equal? ecs-node this-node-id) 
       (list
        ;; module to create test signals
        (make-module 'motion-control "" admin-priority
                     'set-timing display-timing

                     ;; writes AudioObjectMotion, label mosquito
		     'add-moving-sound "mosquito"
		     'set-coordinates 0 0 0  0 0 0  1 0 0  0 0 0.5
		     'set-dt 0.1

                     ;; writes AudioObjectFixed, label ...
                     'add-fixed-sound "rpm1left"
                     'add-fixed-sound "rpm2left"
                     'add-fixed-sound "rpm1right"
                     'add-fixed-sound "rpm2right"
                     'add-fixed-sound "gearup"
                     'event-interval 50
                     'add-fixed-sound "geardown"
                     'event-interval 53
                     'add-fixed-sound "shutdown_left"
                     'event-interval 59
                     'add-fixed-sound "shutdown_right"
                     'event-interval 43
                     'add-fixed-sound "damage"
                     'event-interval 19
                     'add-fixed-sound "mass"
                     'event-interval 61
                     'add-fixed-sound "flaps"
                     'event-interval 64
                     'add-fixed-sound "wind"
                     'add-fixed-sound "wind_gear"
                     'add-fixed-sound "stall"
                     'event-interval 66
                     'add-fixed-sound "overspeed"
                     'event-interval 69
                     'add-fixed-sound "wheels"
                     'event-interval 71
                     'add-fixed-sound "touchdown"
                     'event-interval 74
                    ; 'add-fixed-sound "gearalert"
                   ;  'event-interval 50
		     )
        (make-module 'world-listener "" sim-priority
                     'set-timing display-timing
                     'check-timing 10000 20000
		     'control-logger "HDFLogConfig://ph-sound"
                     'set-listener
                     (make-openal-listener
;		      'set-devicename "CMI8788 [Oxygen HD Audio] (CMI8786 (Xonar DG)) Analog Stereo"
;		      'set-devicename "Xonar DX, Multichannel (CARD=DX,DEV=0)"
;                     'add-controlled-moving-sound "mosquito" "mosquito.wav"
;		      'set-devicename "Xonar DX, Multichannel (CARD=DX,DEV=0)"
;		      'set-devicename "CMI8788 [Oxygen HD Audio] (CMI8786 (Xonar DG)) Analog Stereo"
                      ;'add-controlled-moving-sound
                      ;"mosquito" "mosquito.wav"
                                        ;'set-pitch 1.0

                      ;; the following will match the AudioObjectMotion
                      ;; channel with the mosquito label, give it a
                      ;; name "mosquito #somenumber"
                      ;; create an OpenALObjectMoving object to handle it
                      ;; with the mosquito.wav file
                      'add-object-class-data
                      "AudioObjectMotion:mosquito" "mosquito #"
                      "OpenALObjectMoving" "mosquito.wav"
                      ;; coordinates are x/y/z, u/v/w, volume, pitch
                      'add-object-class-coordinates
                      0 0 0 0 0 0 0.99 1.0

                      ;; old style hacks, link to the label
                      'add-controlled-static-sound
                      "rpm1left" "PA34_rpm1_left.wav"
                      'set-coordinates 0 -2 0 0 0 0

                      'add-controlled-static-sound
                      "rpm1right" "PA34_rpm1_right.wav"
                      'set-coordinates 0 2 0 0 0 0

                      'add-controlled-static-sound
                      "rpm2left" "PA34_rpm2_left.wav"
                      'set-coordinates 0 -2 0 0 0 0

                      'add-controlled-static-sound
                      "rpm2right" "PA34_rpm2_right.wav"
                      'set-coordinates 0 2 0 0 0 0
                      
                      'add-controlled-static-sound
                      "gearup" "PA34_gear_up.wav"
                      'set-coordinates 0 0 -2 0 0 0
                      
                      'add-controlled-static-sound
                      "geardown" "PA34_gear_down.wav"
                      'set-coordinates 0 0 -2 0 0 0
                      
                      'add-controlled-static-sound
                      "shutdown_left" "PA34_shutdown_left.wav"
                      'set-coordinates 0 -2 0 0 0 0

                      'add-controlled-static-sound
                      "shutdown_right" "PA34_shutdown_right.wav"
                      'set-coordinates 0 2 0 0 0 0

                      'add-controlled-static-sound
                      "damage" "damage.wav"
                      'set-coordinates 0 0 0 0 0 0

                      'add-controlled-static-sound
                      "mass" "PA34_mass.wav"
                      'set-coordinates -2 0 0 0 0 0

                      ;; flaps sound is defective file
                      'add-controlled-static-sound
                      "flaps" "PA34_flaps.wav"
                      'set-coordinates 0 0 0 0 0 0

                      'add-controlled-static-sound
                      "wind" "PA34_wind.wav"
                      'set-coordinates 0 0 0 0 0 0
                      'set-looping #t

                      'add-controlled-static-sound
                      "wind_gear" "PA34_wind_gear.wav"
                      'set-coordinates 0 0 -2 0 0 0

                      'add-controlled-static-sound
                      "stall" "PA34_stall.wav"
                      'set-coordinates 0 0 0 0 0 0

                      'add-controlled-static-sound
                      "overspeed" "PA34_overspeed.wav"
                      'set-coordinates 0 0 0 0 0 0

                      'add-controlled-static-sound
                      "wheels" "PA34_groundroll.wav"
                      'set-coordinates 0 0 0 0 0 0

                      'add-controlled-static-sound
                      "touchdown" "PA34_touchdown.wav"
                      'set-coordinates 0 0 -2 0 0 0

                    ;  'add-controlled-static-sound
                    ;  "gearalert" "PA34_gearalert.wav"
                    ;  'set-coordinates 0 0 0 0 0 0
                    ;  'set-looping #t
                      
                      )
                     'initial-ears 100 0 0 0 0 0 0 0 0
                     )
        (make-module 'hdf5-logger "" admin-priority
                     'set-timing sim-timing
                     'chunksize 1000
                     'compress #f
		     'config-channel "HDFLogConfig://ph-sound"
                     'watch-channel "AnyAudioClass://audio" "/data/audio"
                     )
        )
       )
               ; an empty list; at least one list should be supplied
               ; for nodes that have no modules as argument
   (list)
   )
  )

