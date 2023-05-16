#include "Gemm.h"

#include "../Model.h"
#include "../Tensor.h"

Gemm::Gemm(SimulationConfig config, Model* model, onnx::NodeProto& node_proto)
    : Operation(config, model, node_proto) {
  Mdim = 1;
  Cdim_w = 0;
  Cdim = 1;
  Ndim = 0;

  for (auto attribute : node_proto.attribute()) {
    if (attribute.name() == "alpha") {
      _alpha = attribute.f();
    } else if (attribute.name() == "beta") {
      _beta = attribute.f();
    } else if (attribute.name() == "transA") {
      _transA = attribute.i();
      if (_transA) {
        Cdim = 0;
        Ndim = 1;
      }
    } else if (attribute.name() == "transB") {
      _transB = attribute.i();
      if (_transB) {
        Mdim = 0;
        Cdim_w = 1;
      }
    }
  }

  _input_shape = get_input(0)->get_dims();
  _weight_shape = get_input(1)->get_dims();
  std::vector<uint32_t> bias_shape = get_input(2)->get_dims();
  _output_shape.resize(2);
  _output_shape[Ndim] = _input_shape[Ndim];
  _output_shape[Cdim] = _weight_shape[Mdim];

  spdlog::trace("output_shape : {}", _output_shape);
  assert(bias_shape[0] == _output_shape[Cdim]);

  Tensor* pre_defind_tensor = _model->find_tensor(node_proto.output(0));
  if (pre_defind_tensor == nullptr) {
    std::unique_ptr<Tensor> output_tensor = std::make_unique<Tensor>(
        _id, node_proto.output(0), _output_shape, false);
    _outputs.push_back(output_tensor.get()->get_id());
    _model->add_tensor(std::move(output_tensor));
  } else {
    pre_defind_tensor->redefine_tensor(_id, _output_shape);
  }
}

Gemm::Gemm(SimulationConfig config, MappingTable mapping_table,
           std::vector<uint32_t> input_shape,
           std::vector<uint32_t> weight_shape,
           std::vector<uint32_t> output_shape)
    : Operation(config, mapping_table) {
  Mdim = 1;
  Cdim_w = 0;
  Cdim = 1;
  Ndim = 0;
  _input_shape = input_shape;
  _weight_shape = weight_shape;
  _output_shape = output_shape;
}