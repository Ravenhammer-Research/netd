/*
 * Copyright (c) 2024 Paige Thompson / Ravenhammer Research (paige@paige.bio)
 * 
 * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <shared/include/route.hpp>
#include <shared/include/yang.hpp>
#include <libyang/tree_data.h>

namespace netd::shared {

	Route::Route(std::shared_ptr<Address> destination, std::shared_ptr<Address> gateway, 
	             const std::string& interface, uint32_t vrf)
		: destination_(destination), gateway_(gateway), interface_(interface), vrf_(vrf) {
	}

	lyd_node* Route::toYang(ly_ctx* ctx) const {
		// TODO: Implement YANG serialization for routes
		if (!ctx) {
			return nullptr;
		}
		
		// Placeholder implementation - needs actual YANG node creation
		return nullptr;
	}

	Route Route::fromYang(const ly_ctx* ctx, const lyd_node* node) {
		// TODO: Implement YANG deserialization for routes
		if (!ctx || !node) {
			return Route();
		}
		
		// Placeholder implementation - needs actual YANG node parsing
		return Route();
	}

	std::shared_ptr<Address> Route::getDestination() const {
		return destination_;
	}

	std::shared_ptr<Address> Route::getGateway() const {
		return gateway_;
	}

	std::string Route::getInterface() const {
		return interface_;
	}

	uint32_t Route::getVRF() const {
		return vrf_;
	}

	void Route::setDestination(std::shared_ptr<Address> destination) {
		destination_ = destination;
	}

	void Route::setGateway(std::shared_ptr<Address> gateway) {
		gateway_ = gateway;
	}

	void Route::setInterface(const std::string& interface) {
		interface_ = interface;
	}

	void Route::setVRF(uint32_t vrf) {
		vrf_ = vrf;
	}

} // namespace netd::shared
