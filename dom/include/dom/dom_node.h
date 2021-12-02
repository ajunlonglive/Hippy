#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "dom/dom_listener.h"
#include "dom/dom_manager.h"
#include "dom/dom_value.h"
#include "dom/taitank_layout_node.h"

namespace hippy {
inline namespace dom {

class LayoutNode;
class DomManager;

class DomNode : public std::enable_shared_from_this<DomNode> {
 public:
  using DomValue = tdf::base::DomValue;

  DomNode(int32_t id, int32_t pid, int32_t index, std::string tag_name, std::string view_name,
          std::unordered_map<std::string, std::shared_ptr<DomValue>>&& style_map,
          std::unordered_map<std::string, std::shared_ptr<DomValue>>&& dom_ext_map,
          const std::shared_ptr<DomManager>& dom_manager);
  DomNode(int32_t id, int32_t pid, int32_t index);
  ~DomNode();

  // 记录RenderNode相关信息
  struct RenderInfo {
    int32_t pid = -1;      // 父RenderNode的id
    int32_t index = -1;    // 在父RenderNode上的索引值
    bool created = false;  // RenderNode是否已经创建

    void Reset() {
      pid = -1;
      index = -1;
      created = false;
    }
  };

  inline std::shared_ptr<DomNode> GetParent() { return parent_.lock(); }
  inline void SetParent(std::shared_ptr<DomNode> parent) { parent_ = parent; }
  inline int32_t GetChildCount() const { return children_.size(); }
  inline void SetTagName(const std::string& tag_name) { tag_name_ = tag_name; }
  inline const std::string& GetTagName() { return tag_name_; }
  inline const std::string& GetViewName() { return view_name_; }
  inline std::shared_ptr<LayoutNode> GetLayoutNode() {return layout_node_; }
  inline void SetId(int32_t id) { id_ = id; }
  inline int32_t GetId() const { return id_; }
  inline void SetPid(int32_t pid) { pid_ = pid; }
  inline int32_t GetPid() const { return pid_; }
  inline RenderInfo GetRenderInfo() const { return render_info_; }
  inline void SetRenderInfo(RenderInfo render_info) { render_info_ = render_info; }
  inline bool IsJustLayout() const { return is_just_layout_; }
  inline void SetIsJustLayout(bool is_just_layout) { is_just_layout_ = is_just_layout; }
  inline bool IsVirtual() { return is_virtual_; }
  inline void SetIsVirtual(bool is_virtual) { is_virtual_ = is_virtual; }
  inline void SetIndex(int32_t index) { index_ = index; }
  inline int32_t GetIndex() const { return index_; }

  void SetLayoutWidth(float width);
  void SetLayoutHeight(float height);
  int32_t IndexOf(const std::shared_ptr<DomNode>& child);
  std::shared_ptr<DomNode> GetChildAt(int32_t index);
  void AddChildAt(const std::shared_ptr<DomNode>& dom_node, int32_t index);
  std::shared_ptr<DomNode> RemoveChildAt(int32_t index);
  void DoLayout();
  void ParseLayoutStyleInfo();
  void TransferLayoutOutputsRecursive();
  int32_t AddClickEventListener(OnClickEventListener listener);
  void RemoveClickEventListener(int32_t listener_id);
  int32_t AddLongClickEventListener(OnLongClickEventListener listener);
  void RemoveLongClickEventListener(int32_t listener_id);
  int32_t AddTouchEventListener(TouchEvent event, OnTouchEventListener listener);
  void RemoveTouchEventListener(TouchEvent event, int32_t listener);
  int32_t SetOnAttachChangedListener(OnAttachChangedListener listener);
  int32_t AddShowEventListener(ShowEvent event, OnShowEventListener listener);
  void RemoveShowEventListener(ShowEvent event, int32_t listener_id);
  void CallClick();
  void CallLongClick();
  void CallTouch(TouchEvent event, TouchEventInfo info);
  void CallOnShow(ShowEvent event);

  std::tuple<int32_t, int32_t> GetSize();
  void SetSize(int32_t width, int32_t height);
  int32_t AddDomEventListener(DomEvent event, OnDomEventListener listener);
  void RemoveDomEventListener(DomEvent event, int32_t listener_id);
  int32_t AddOnLayoutListener(LayoutEvent event, OnLayoutEventListener listener);
  void RemoveOnLayoutListener(LayoutEvent event, int32_t listener_id);
  void OnDomNodeStateChange(DomEvent event);
  const std::unordered_map<std::string, std::shared_ptr<DomValue>>& GetStyleMap() const { return style_map_; }

  bool HasTouchEventListeners() const { return !touch_listeners->empty(); }
  void CallFunction(const std::string& name,
                    std::unordered_map<std::string, std::shared_ptr<DomValue>> param,
                    const CallFunctionCallback& cb);
  const std::unordered_map<std::string, std::shared_ptr<DomValue>> GetStyle() { return style_map_; }
  const std::unordered_map<std::string, std::shared_ptr<DomValue>> GetExtStyle() { return dom_ext_map_; }
  const std::unordered_map<std::string, std::shared_ptr<DomValue>> GetDiffStyle() { return diff_; }
  void SetDiffStyle(std::unordered_map<std::string, std::shared_ptr<DomValue>> diff) { diff_ = std::move(diff); }

  CallFunctionCallback GetCallback(const std::string& name);

 protected:
  void OnLayout(LayoutEvent event, LayoutResult result);

 private:
  int32_t id_;             // 节点唯一id
  int32_t pid_;            // 父节点id
  int32_t index_;          // 当前节点在父节点孩子数组中的索引位置
  std::string tag_name_;   // DSL 中定义的组件名称
  std::string view_name_;  // 底层映射的组件
  std::unordered_map<std::string, std::shared_ptr<DomValue>> style_map_;
  // 样式预处理后结果
  std::unordered_map<std::string, std::shared_ptr<DomValue>> dom_ext_map_;
  //  用户自定义数据
  std::unordered_map<std::string, std::shared_ptr<DomValue>> diff_;
  // Update 时用户自定义数据差异，UpdateRenderNode 完成后会清空 map，以节省内存

  std::shared_ptr<LayoutNode> layout_node_;
  LayoutResult layout_;  // Layout 结果
  bool is_just_layout_;
  bool is_virtual_;

  OnLayoutEventListener on_layout_event_listener_;

  std::weak_ptr<DomNode> parent_;
  std::vector<std::shared_ptr<DomNode>> children_;

  RenderInfo render_info_;
  std::weak_ptr<DomManager> dom_manager_;
  int32_t current_callback_id_;
  std::shared_ptr<std::unordered_map<DomEvent, std::unordered_map<int32_t, OnDomEventListener>>> dom_event_listeners;
  std::shared_ptr<std::unordered_map<LayoutEvent, std::unordered_map<int32_t, OnLayoutEventListener>>> layout_listeners;
  std::shared_ptr<std::unordered_map<int32_t, OnTouchEventListener>> touch_listeners;
  std::shared_ptr<std::unordered_map<int32_t, OnClickEventListener>> click_listeners;
  std::shared_ptr<std::unordered_map<int32_t, OnLongClickEventListener>> long_click_listeners;
  std::shared_ptr<std::unordered_map<int32_t, OnShowEventListener>> show_listeners;
  std::shared_ptr<std::unordered_map<std::string, CallFunctionCallback>> callbacks_;
};

}  // namespace dom
}  // namespace hippy