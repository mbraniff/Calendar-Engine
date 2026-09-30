#include "app/containers/vstack.hpp"
#include "app/graphics.hpp"
#include <cstddef>
#include <numeric>

  std::vector<Size> VStack::childSizes(Size proposal) {
      size_t n = subviews_.size();
      std::vector<Size> sizes(n);
      if (n == 0) return sizes;

      const float inf = std::numeric_limits<float>::infinity();
      std::vector<float> flex(n);
      for (size_t i = 0; i < n; ++i) {
          float lo = subviews_[i]->sizeThatFits({ proposal.width, 0 }).height;
          float hi = subviews_[i]->sizeThatFits({ proposal.width, inf }).height;
          flex[i] = hi - lo;
      }

      std::vector<size_t> order(n);
      std::iota(order.begin(), order.end(), 0);
      std::stable_sort(order.begin(), order.end(),
                       [&](size_t a, size_t b) { return flex[a] < flex[b]; });

      float remaining = std::max(0.0f, proposal.height - spacing * (n - 1));
      size_t left = n;
      for (size_t i : order) {
          Size s = subviews_[i]->sizeThatFits({ proposal.width, remaining / left });
          sizes[i] = s;
          remaining = std::max(0.0f, remaining - s.height);
          --left;
      }
      return sizes;
  }

  Size VStack::measure(Size proposal) {
      auto sizes = childSizes(proposal);
      Size total{ 0, 0 };
      for (auto& s : sizes) {
          total.width = std::max(total.width, s.width);
          total.height += s.height;
      }
      if (!sizes.empty()) total.height += spacing * (sizes.size() - 1);
      return total;
  }

  void VStack::layoutSubviews() {
      Rect content = contentFrame();
      auto sizes = childSizes({ content.width, content.height });
      float y = content.origin.y;
      for (size_t i = 0; i < subviews_.size(); ++i) {
          Size s = sizes[i];
          float x = content.origin.x + (content.width - s.width) * layoutAlignment.x;
          subviews_[i]->layout({ { x, y }, s.width, s.height });
          y += s.height + spacing;
      }
  }