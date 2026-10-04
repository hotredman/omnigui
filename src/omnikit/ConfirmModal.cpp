#include "omnikit/ConfirmModal.hpp"

#include "omnikit/Text.hpp"
#include "omnikit/Modal.hpp"

namespace OmniKit {

ConfirmModal::ConfirmModal(const ConfirmModalOptions& options)
    : m_title(options.title ? options.title : ""),
      m_message(options.message ? options.message : ""),
      m_confirmLabel(options.confirmLabel ? options.confirmLabel : "Confirm"),
      m_variant(options.variant) {}

bool ConfirmModal::Render() {
    if (!m_open) return false;

    // Ширина 460px, авто-высота, без подложки карточки
    Modal modal(m_open, {.title = m_title.c_str(), .width = 460.0f, .height = 0.0f, .cardBackground = false});
    if (!modal) return false;

    if (auto header = modal.Header()) {
        header.Title(m_title);
    }
    if (auto content = modal.Content({.scroll = ModalScroll::None})) {
        OmniKit::Text(m_message, {.variant = UiVariant::Secondary, .wrap = true});
    }
    if (auto footer = modal.Footer()) {
        if (footer.Actions({.confirmLabel = m_confirmLabel.c_str(), .variant = m_variant, .icon = Icon::None})) {
            return true;
        }
    }
    return false;
}

} // namespace OmniKit
