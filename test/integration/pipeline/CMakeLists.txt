addtest(tx_pipeline_integration_test tx_pipeline_integration_test.cpp)
target_link_libraries(tx_pipeline_integration_test
    application
    raw_block_insertion
    model_generators
    shared_model_proto_backend
    shared_model_ed25519_sha3
    shared_model_stateless_validation
    )

addtest(transfer_asset_inter_domain_test transfer_asset_inter_domain_test.cpp)
target_link_libraries(transfer_asset_inter_domain_test
  application
  raw_block_insertion
  model_generators
  )
