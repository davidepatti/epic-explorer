// Artificial Neural Network Approximation 
#include "FannApprox.h"
#include <fann.h>

  CAnnFunctionApproximation::CAnnFunctionApproximation() {
  
  };
  
  CAnnFunctionApproximation::~CAnnFunctionApproximation() {
  
  //fann_destroy_train(train_data);
  //fann_destroy_train(test_data);
  fann_save(ann,"ANN_file.txt");
  fann_destroy(ann);
  if (train_data != NULL) fann_destroy_train(train_data);
  fclose(train_file);
  fclose(test_file);
  fclose(log_file);
  
  };

  bool CAnnFunctionApproximation::Init(double _threshold, int min, int max, int nouts) {
  
  cout<< "ANN Init\n";
  
  num_output = 2;//nouts;
  desired_error = _threshold;
  min_sims = min;
  max_sims = max;
  num_input = 18;
  max_epochs = 100;
  num_layers = 3;
  simulations_done = 0;
  train_ = 0;
  test_ = 0;
  trained = true;
  train_data = NULL;
  
  train_file = fopen("train.data","w+");
  test_file = fopen("test.data","w+");
  log_file = fopen("ann.log","w");
  
  return (StartUp(max_epochs,desired_error,min_sims,max_sims));
  
  };
  
  bool CAnnFunctionApproximation::StartUp(int MaxNumberOfEpochs, double threshold, int _min, int _max) {
    
  cout<< "ANN StartUp\n";
    
    ann = fann_create_standard(num_layers, num_input, num_input, num_output);
    fann_set_training_algorithm(ann, FANN_TRAIN_RPROP);
    fann_set_activation_function_layer(ann, FANN_LINEAR,0);
    fann_set_activation_steepness_layer(ann, 1.0f,0);
    fann_set_activation_function_hidden(ann, FANN_SIGMOID);
    fann_set_activation_steepness_hidden(ann, 1.0f);
    fann_set_activation_function_output(ann, FANN_LINEAR);
    fann_set_activation_steepness_output(ann, 1.0f);

  fprintf(train_file,"%*u %u %u", 4,train_,num_input,num_output);
  fprintf(test_file,"%*u %u %u", 4,test_,num_input,num_output);
  
  fflush(train_file);
  fflush(test_file);


  };
  
  bool CAnnFunctionApproximation::Learn(Configuration conf,Simulation sim,Processor& p,Mem_hierarchy& mem) {
  
  
  cout << "\nANN is learning\n";
  
  simulations_done++;
  
  FILE *wrapper_file;
  unsigned wrapper_number;
  
  unsigned in_appoggio[18];
  in_appoggio[0] = unsigned(p.integer_units.get_pos(conf.integer_units));
  in_appoggio[1] = unsigned(p.float_units.get_pos(conf.float_units));
  in_appoggio[2] = unsigned(p.branch_units.get_pos(conf.branch_units));
  in_appoggio[3] = unsigned(p.memory_units.get_pos(conf.memory_units));
  in_appoggio[4] = unsigned(p.gpr_static_size.get_pos(conf.gpr_static_size));
  in_appoggio[5] = unsigned(p.fpr_static_size.get_pos(conf.fpr_static_size));
  in_appoggio[6] = unsigned(p.pr_static_size.get_pos(conf.pr_static_size));
  in_appoggio[7] = unsigned(p.cr_static_size.get_pos(conf.cr_static_size));
  in_appoggio[8] = unsigned(p.btr_static_size.get_pos(conf.btr_static_size));
  in_appoggio[9] = unsigned(mem.L1D.size.get_pos(conf. L1D_size  ));
  in_appoggio[10] = unsigned(mem.L1D.block_size.get_pos(conf.L1D_block));
  in_appoggio[11] = unsigned(mem.L1D.associativity.get_pos(conf.L1D_assoc));
  in_appoggio[12] = unsigned(mem.L1I.size.get_pos(conf.L1I_size  ));
  in_appoggio[13] = unsigned(mem.L1I.block_size.get_pos(conf.L1I_block));
  in_appoggio[14] = unsigned(mem.L1I.associativity.get_pos(conf.L1I_assoc));
  in_appoggio[15] = unsigned(mem.L2U.size.get_pos(conf.L2U_size  ));
  in_appoggio[16] = unsigned(mem.L2U.block_size.get_pos(conf.L2U_block));
  in_appoggio[17] = unsigned(mem.L2U.associativity.get_pos(conf.L2U_assoc));
  in_appoggio[18] = unsigned(p.num_clusters.get_pos(conf.num_clusters));
  
  fann_type out_appoggio[2];
  out_appoggio[0] = fann_type(sim.energy); 
  out_appoggio[1] = fann_type(sim.exec_time);
  
  if ((simulations_done%2) || (simulations_done > max_sims) || (simulations_done < 3)) {
  	wrapper_file = train_file;
	train_++;
	wrapper_number = train_;
	trained = false;
  } else {
  	wrapper_file = test_file;
	test_++;
	wrapper_number = test_;
  }
    
  fseek(wrapper_file,0,SEEK_SET );
  
  fprintf(wrapper_file,"%*u %u %u\n", 4,wrapper_number,num_input,num_output);
   
  fflush(wrapper_file);
  
  fseek(wrapper_file, 0, SEEK_END);
  
  for (i=0;i<num_input;++i)
  	fprintf(wrapper_file,"%u ", in_appoggio[i]);
	 
  fprintf(wrapper_file,"\n");
  fflush(wrapper_file);
  
  for (i=0;i<num_output;++i)
  	fprintf(wrapper_file,"%f ", out_appoggio[i]); 
  
  fprintf(wrapper_file,"\n");
  fflush(wrapper_file);
  
  if (simulations_done%2 || (simulations_done > max_sims)) {
  	if (train_data != NULL) fann_destroy_train(train_data);
  	train_data = fann_read_train_from_file("train.data");
	fann_set_input_scaling_params(ann,train_data,-1,1);
 	fann_scale_train(ann,train_data);
	}
  
  };
  
  Simulation CAnnFunctionApproximation::Estimate1(Configuration conf,Processor& p,Mem_hierarchy& mem) {
  
    	if (!trained) {
   		fann_train_on_data(ann,train_data,max_epochs,20,0);
		trained = true;
	}
	
	Simulation sim;
	
	fann_type in_appoggio[18];
	in_appoggio[0] = fann_type(p.integer_units.get_pos(conf.integer_units));
	in_appoggio[1] = fann_type(p.float_units.get_pos(conf.float_units));
	in_appoggio[2] = fann_type(p.branch_units.get_pos(conf.branch_units));
 	in_appoggio[3] = fann_type(p.memory_units.get_pos(conf.memory_units));
  	in_appoggio[4] = fann_type(p.gpr_static_size.get_pos(conf.gpr_static_size));
  	in_appoggio[5] = fann_type(p.fpr_static_size.get_pos(conf.fpr_static_size));
  	in_appoggio[6] = fann_type(p.pr_static_size.get_pos(conf.pr_static_size));
  	in_appoggio[7] = fann_type(p.cr_static_size.get_pos(conf.cr_static_size));
  	in_appoggio[8] = fann_type(p.btr_static_size.get_pos(conf.btr_static_size));
  	in_appoggio[9] = fann_type(mem.L1D.size.get_pos(conf. L1D_size  ));
  	in_appoggio[10] = fann_type(mem.L1D.block_size.get_pos(conf.L1D_block));
  	in_appoggio[11] = fann_type(mem.L1D.associativity.get_pos(conf.L1D_assoc));
  	in_appoggio[12] = fann_type(mem.L1I.size.get_pos(conf.L1I_size  ));
  	in_appoggio[13] = fann_type(mem.L1I.block_size.get_pos(conf.L1I_block));
  	in_appoggio[14] = fann_type(mem.L1I.associativity.get_pos(conf.L1I_assoc));
  	in_appoggio[15] = fann_type(mem.L2U.size.get_pos(conf.L2U_size  ));
  	in_appoggio[16] = fann_type(mem.L2U.block_size.get_pos(conf.L2U_block));
  	in_appoggio[17] = fann_type(mem.L2U.associativity.get_pos(conf.L2U_assoc));
	in_appoggio[18] = fann_type(p.num_clusters.get_pos(conf.num_clusters));

	fann_type *out_appoggio;

	fann_scale_input(ann,in_appoggio);
	
     	out_appoggio = fann_run(ann,in_appoggio);

	sim.config = conf;
	sim.area = -1.0f;
	sim.clock_freq = 0.0f;
	sim.energy = double(out_appoggio[0]);
	sim.exec_time = double(out_appoggio[1]);
	sim.simulated = false;
	cout << "\n-----------Estimated energy : " << sim.energy << " __ execution time : " << sim.exec_time;
	fprintf(log_file,"Estimation : %f %f\n",out_appoggio[0],out_appoggio[1]);
	fflush(log_file);
    	return (sim);
  
  
  };

  bool CAnnFunctionApproximation::Reliable() {
  
  if (simulations_done < min_sims)
  	return (false);
  
  if (simulations_done > max_sims)
  	return (true);
	
  if (!trained) {
 	fann_train_on_data(ann,train_data,max_epochs,20,0);
	trained = true;
  }
	
   test_data = fann_read_train_from_file("test.data");
	
   fann_scale_train(ann,test_data);
   
   float errore = (fann_test_data(ann, test_data) * 100);
   
   cout << "\n Approximation error is : "<< errore << " - objective is "<< desired_error;
   
   fann_destroy_train(test_data);
   
   fprintf(log_file, "Reliability tested, error is %f (percentage) after %u simulations\n",errore,simulations_done);
   fflush(log_file);
   
   if (errore > desired_error)
   	return (false);
	
   return (true);
  
  
  };

  int CAnnFunctionApproximation::GetSystem() {
  return 0;
  };

  void CAnnFunctionApproximation::Clean() {
  
  fann_destroy(ann);
  fclose(train_file);
  fclose(test_file);
  simulations_done = 0;
  train_ = 0;
  test_ = 0;
  trained = true;
  
  };





bool CAnnFunctionApproximation::Learn(const Configuration& conf,const Dynamic_stats& dyn) {};
Dynamic_stats CAnnFunctionApproximation::Estimate2(Configuration conf) {};
  
