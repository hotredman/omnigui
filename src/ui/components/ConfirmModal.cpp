#include "ui/components/ConfirmModal.hpp"

#include "ui/components/Text.hpp"
#include "ui/components/Modal.hpp"

ConfirmModal::ConfirmModal(std::string id, std::string title, std::string text,
                           std::string confirmLabel, UiVariant variant)
    : m_windowName(title + "##" + id),
      m_title(std::move(title)),
      m_text(std::move(text)),
      m_confirmLabel(std::move(confirmLabel)),
      m_variant(variant) {}

bool ConfirmModal::Render() {
    if (!m_open) return false;

    // Ширина 460px, авто-высота, без подложки карточки
    Modal modal(m_windowName.c_str(), m_open, Modal::Config(460.0f, 0.0f, 0.0f, false));
    if (!modal) return false;

    if (auto header = modal.Header()) {
        header.Title(m_title.c_str());
    }
    if (auto content = modal.Content(ModalScroll::None)) {
        ::Text(m_text, {.variant = UiVariant::Secondary, .wrap = true});
    }
    if (auto footer = modal.Footer()) {
        if (footer.Actions(m_confirmLabel.c_str(), "Cancel", true, m_variant, Icon::None)) {
            return true;
        }
    }
    return false;
}
